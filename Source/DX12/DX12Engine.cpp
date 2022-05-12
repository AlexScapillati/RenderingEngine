#include "DX12Engine.h"

#include <filesystem>

#include "D3D12Helpers.h"
#include "DX12ConstantBuffer.h"
#include "DX12DescriptorHeap.h"
#include "DX12Gui.h"
#include "DX12PipelineObject.h"
#include "DX12Scene.h"
#include "DX12Shader.h"
#include "DX12Texture.h"
#include "../Window.h"
#include "../Common/CGameObjectManager.h"
#include "Objects/CDX12Sky.h"
#include "Objects/DX12DirectionalLight.h"
#include "Objects/DX12Light.h"
#include "Objects/DX12PointLight.h"
#include "Objects/DX12SpotLight.h"

#include "dxgi1_6.h"
#include "dxgidebug.h"

namespace DX12
{
	CDX12Engine::CDX12Engine(HINSTANCE hInstance,
		int       nCmdShow)
	{

		// Prepare TL-Engine style input functions
		InitInput();

		//get the executable path
		CHAR path[MAX_PATH];

		GetModuleFileNameA(hInstance, path, MAX_PATH);

		const auto pos = std::string(path).find_last_of("\\/");

		//get the media folder
		mMediaFolder = std::string(path).substr(0, pos) + "\\Media\\";

		//get the shader folder
		mShaderFolder = std::string(path).substr(0, pos) + "\\Source\\Shaders\\";

		mMediaFolder = ReplaceAll(mMediaFolder, std::string("\\"), std::string("/"));
		mShaderFolder = ReplaceAll(mShaderFolder, std::string("\\"), std::string("/"));

		try
		{
			// Create a window 
			mWindow = std::make_unique<CWindow>(hInstance, nCmdShow);

			// Initialise Direct3D
			InitD3D();

			// Load Shaders
			LoadDefaultShaders();

			CreatePipelineStateObjects();

			InitFrameDependentResources();


			mObjManager = std::make_unique<CGameObjectManager>(this);

			// Create Gui
			mGui = std::make_unique<CDX12Gui>(this);
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error(e.what());
		}

		// Will use a timer class to help in this tutorial (not part of DirectX). It's like a stopwatch - start it counting now
		mTimer.Start();
	}

	CDX12Engine::~CDX12Engine() { Flush(); }

	bool CDX12Engine::Update()
	{
		// Main message loop - this is a Windows equivalent of the loop in a TL-Engine application
		MSG msg = {};
		while (msg.message != WM_QUIT) // As long as window is open
		{
			// Check for and deal with any window messages (input, window resizing, minimizing, etc.).
			// The actual message processing happens in the function WndProc below
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				// Deal with messages
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else // When no windows messages left to process then render & update our scene
			{
				// Update the scene by the amount of time since the last frame
				auto frameTime = mTimer.GetLapTime();

				if (mRaytracing)
				{
					InitializeFrame();
					MidFrame();
					RaytracingFrame();

					mScene->RenderScene(frameTime);
					mScene->UpdateScene(frameTime);

					mGui->Begin();

					mGui->Show(frameTime);

					mGui->End();

					FinalizeFrame();
					Present();
				}
				else
					try
				{
					InitializeFrame();


					// Draw the scene
					mScene->RenderScene(frameTime);

					mScene->UpdateScene(frameTime);

					MidFrame();

					PIXBeginEvent(mCommandList.Get(), 0, L"GUI Rendering");

					mGui->Begin();

					//mGui
					mGui->Show(frameTime);

					mGui->End();

					PIXEndEvent(mCommandList.Get());

					FinalizeFrame();

					Present();
				}
				catch (const std::exception& e) { throw std::runtime_error(e.what()); }

				if (KeyHit(Key_Space)) mRaytracing = !mRaytracing;

				if (KeyHit(Key_Escape))
				{
					// Save automatically
					//mMainScene->Save();

					Flush();

					CloseHandle(mFenceEvent);

					return false;
				}
			}
		}

		Flush();

		CloseHandle(mFenceEvent);

		return (int)msg.wParam;
	}


	void CDX12Engine::MidFrame()
	{
		mSRVDescriptorHeap->Set();

		// Set the viewport
		mCurrRecordingCommandList->RSSetViewports(1, &mViewport);
		mCurrRecordingCommandList->RSSetScissorRects(1, &mScissorRect);

		// Set and clear the render target.
		mBackBuffers[mCurrentBackBufferIndex]->Barrier(D3D12_RESOURCE_STATE_RENDER_TARGET);

		const FLOAT clearColor[] = { 0.4f,0.6f,0.9f,1.0f };

		auto rtv = mBackBuffers[mCurrentBackBufferIndex]->mRTVHandle->mCpu;

		mCurrRecordingCommandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
		mCurrRecordingCommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
	}

	void CDX12Engine::InitializeFrame()
	{
		mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

		// Reset the main command allocator and the command list
		mCommandAllocators[mCurrentBackBufferIndex]->Reset();
		ThrowIfFailed(mCommandList->Reset(mCommandAllocators[mCurrentBackBufferIndex].Get(), nullptr));
	}

	void CDX12Engine::FinalizeFrame()
	{
		// Present
		{
			mBackBuffers[mCurrentBackBufferIndex]->Barrier(D3D12_RESOURCE_STATE_PRESENT);

			mCommandList->Close();
			
			std::vector<ID3D12CommandList*> commandLists;
			commandLists.push_back(mCommandList.Get());
			mCommandQueue->ExecuteCommandLists(commandLists.size(), commandLists.data());
		}
	}

	void CDX12Engine::Present()
	{
		mFrameFenceValues[mCurrentBackBufferIndex] = Signal();

		DXGI_FRAME_STATISTICS o;
		mSwapChain->GetFrameStatistics(&o);

		DXGI_PRESENT_PARAMETERS parameters;
		parameters.DirtyRectsCount = 0;
		parameters.pDirtyRects = nullptr;
		parameters.pScrollRect = nullptr;
		parameters.pScrollOffset = nullptr;

		const UINT syncInterval = mScene->GetLockFps() ? 1 : 0;
		const UINT presentFlags = !mScene->GetLockFps() ? DXGI_PRESENT_ALLOW_TEARING : 0;
		if (FAILED(mSwapChain->Present1(syncInterval, presentFlags, &parameters)))
		{
			throw std::runtime_error("Error presenting");
		}

		mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

		Flush();

	}


	ID3D12Device2* CDX12Engine::GetDevice() const { return mDevice.Get(); }

	ImTextureID CDX12Engine::GetSceneTex() const { return mScene->GetTextureSRV(); }

	void CDX12Engine::CopyBuffers()
	{
		auto i = mCurrentBackBufferIndex;

		mPerFrameConstantBuffer[i]->Copy(mPerFrameConstants[i]);

		mPerFrameLightsConstantBuffer[i]->Copy<PerFrameLights, sLight>(mPerFrameLights[i], mObjManager->mLights.size());
		mPerFrameSpotLightsConstantBuffer[i]->Copy<PerFrameSpotLights, sSpotLight>(mPerFrameSpotLights[i], mObjManager->mLights.size());
		mPerFrameDirLightsConstantBuffer[i]->Copy<PerFrameDirLights, sDirLight>(mPerFrameDirLights[i], mObjManager->mLights.size());
		mPerFramePointLightsConstantBuffer[i]->Copy<PerFramePointLights, sPointLight>(mPerFramePointLights[i], mObjManager->mLights.size());
	}

	void CDX12Engine::UpdateLightsBuffers()
	{
		/// 
		/// Normal lights 
		///

		for (auto i = 0u; i < mObjManager->mLights.size(); ++i)
		{
			sLight lightInfo;
			auto   light = mObjManager->mLights[i];
			lightInfo.position = light->Position();
			lightInfo.enabled = static_cast<float>(*light->Enabled());
			lightInfo.colour = light->GetColour();
			lightInfo.intensity = light->GetStrength();
			mPerFrameLights[mCurrentBackBufferIndex].lights[i] = lightInfo;
		}

		/// 
		/// Spot lights 
		///

		for (auto i = 0u; i < mObjManager->mSpotLights.size(); ++i)
		{
			sSpotLight lightInfo;
			auto       light = mObjManager->mSpotLights[i];
			lightInfo.pos = light->Position();
			lightInfo.enabled = static_cast<float>(*light->Enabled());
			lightInfo.colour = light->GetColour();
			lightInfo.intensity = light->GetStrength();
			lightInfo.facing = Normalise(light->WorldMatrix().GetRow(2));
			lightInfo.cosHalfAngle = cos(ToRadians(light->GetConeAngle() / 2));
			lightInfo.viewMatrix = InverseAffine(light->WorldMatrix());
			lightInfo.projMatrix = MakeProjectionMatrix(1.0f, ToRadians(light->GetConeAngle()));
			mPerFrameSpotLights[mCurrentBackBufferIndex].spotLights[i] = lightInfo;
		}

		/// 
		/// Directional lights 
		///

		for (auto i = 0u; i < mObjManager->mDirLights.size(); ++i)
		{
			sDirLight lightInfo;
			auto       light = mObjManager->mDirLights[i];
			lightInfo.enabled = static_cast<float>(*light->Enabled());
			lightInfo.colour = light->GetColour();
			lightInfo.intensity = light->GetStrength();
			lightInfo.facing = light->Position();
			lightInfo.viewMatrix = InverseAffine(light->WorldMatrix());
			lightInfo.projMatrix = MakeOrthogonalMatrix(light->GetWidth(),
				light->GetWidth(),
				light->GetNearClip(),
				light->GetFarClip());
			mPerFrameDirLights[mCurrentBackBufferIndex].dirLights[i] = lightInfo;
		}

		/// 
		/// Omnidirectional lights 
		///

		for (auto i = 0u; i < mObjManager->mPointLights.size(); ++i)
		{
			auto         light = mObjManager->mPointLights[i];
			sPointLight lightInfo;
			lightInfo.colour = light->GetColour();
			lightInfo.enabled = static_cast<float>(*light->Enabled());
			lightInfo.position = light->Position();
			lightInfo.intensity = light->GetStrength();
			lightInfo.projMatrix = MakeProjectionMatrix(1.0f, ToRadians(90.f));

			for (int j = 0; j < 6; ++j)
			{
				CVector3 rot = light->mSides[j];
				light->SetRotation(rot * PI);
				lightInfo.viewMatrices[j] = InverseAffine(light->WorldMatrix());
			}

			mPerFramePointLights[mCurrentBackBufferIndex].pointLights[i] = lightInfo;
		}

		mPerFrameConstants[mCurrentBackBufferIndex].nLights = static_cast<float>(mObjManager->mLights.size());
		mPerFrameConstants[mCurrentBackBufferIndex].nSpotLights = static_cast<float>(mObjManager->mSpotLights.size());
		mPerFrameConstants[mCurrentBackBufferIndex].nDirLight = static_cast<float>(mObjManager->mDirLights.size());
		mPerFrameConstants[mCurrentBackBufferIndex].nPointLights = static_cast<float>(mObjManager->mPointLights.size());
	}

	void CDX12Engine::SetPBRPSO()
	{
		if (mCurrSetPso == mPbrPso.get()) return;

		mPbrPso->Set();
		mCurrSetPso = mPbrPso.get();
	}

	void CDX12Engine::SetSkyPSO()
	{
		if (mCurrSetPso == mSkyPso.get()) return;

		mSkyPso->Set();
		mCurrSetPso = mSkyPso.get();
	}

	void CDX12Engine::SetDepthOnlyPSO()
	{
		if (mCurrSetPso == mDepthOnlyPso.get()) return;

		mDepthOnlyPso->Set();
		mCurrSetPso = mDepthOnlyPso.get();
	}

	uint64_t CDX12Engine::ExecuteCommandList(ID3D12GraphicsCommandList2* commandList)
	{
		commandList->Close();

		ID3D12CommandAllocator* commandAllocator;
		UINT                    dataSize = sizeof(commandAllocator);
		commandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator);

		ID3D12CommandList* const ppCommandLists[] = { commandList };

		mCommandQueue->ExecuteCommandLists(1, ppCommandLists);
		const uint64_t fenceValue = Signal();

		// The ownership of the command allocator has been transferred to the ComPtr
		// in the command allocator queue. It is safe to release the reference 
		// in this temporary COM pointer here.
		commandAllocator->Release();

		return fenceValue;
	}

	void CDX12Engine::WaitForFenceValue(
		ComPtr<ID3D12Fence>       fence,
		uint64_t                  fenceValue,
		HANDLE                    fenceEvent,
		std::chrono::milliseconds duration)
	{
		if (fence->GetCompletedValue() < fenceValue)
		{
			if (FAILED(fence->SetEventOnCompletion(fenceValue, fenceEvent)))
			{
				throw std::runtime_error("Error waiting for the fence");
			}
			::WaitForSingleObjectEx(fenceEvent, static_cast<DWORD>(duration.count()), FALSE);
		}
	}


	void CDX12Engine::CreateScene(std::string fileName)
	{
		mScene = std::make_unique<CDX12Scene>(this, fileName);
	}


	CGameObject* CDX12Engine::CreateObject(const std::string& mesh,
		const std::string& name,
		const std::string& diffuseMap,
		CVector3           position,
		CVector3           rotation,
		float              scale)
	{
		auto obj = new CDX12GameObject(this, mesh, name, diffuseMap, position, rotation, scale);
		mObjManager->AddObject(obj);
		return obj;
	}

	CSky* CDX12Engine::CreateSky(const std::string& mesh,
		const std::string& name,
		const std::string& diffuseMap,
		CVector3           position,
		CVector3           rotation,
		float              scale)
	{
		auto s = new CDX12Sky(this, mesh, name, diffuseMap, position, rotation, scale);
		mObjManager->AddSky(s);
		return s;
	}

	CPlant* CDX12Engine::CreatePlant(const std::string& id,
		const std::string& name,
		CVector3           position,
		CVector3           rotation,
		float              scale)
	{
		auto p = new CDX12Plant(this, id, name, position, rotation, scale);
		mObjManager->AddPlant(p);
		return p;
	}

	CGameObject* CDX12Engine::CreateObject(const std::string& dirPath,
		const std::string& name,
		CVector3           position,
		CVector3           rotation,
		float              scale)
	{
		auto o = new CDX12GameObject(this, dirPath, name, position, rotation, scale);
		mObjManager->AddObject(o);
		return o;
	}

	CLight* CDX12Engine::CreateLight(const std::string& mesh,
		const std::string& name,
		const std::string& diffuseMap,
		const CVector3& colour,
		const float& strength,
		CVector3           position,
		CVector3           rotation,
		float              scale)
	{
		auto l = new CDX12Light(this, mesh, name, diffuseMap, colour, strength, position, rotation, scale);
		mObjManager->AddLight(l);
		return l;
	}

	CSpotLight* CDX12Engine::CreateSpotLight(const std::string& mesh,
		const std::string& name,
		const std::string& diffuseMap,
		const CVector3& colour,
		const float& strength,
		CVector3           position,
		CVector3           rotation,
		float              scale)
	{
		auto s = new CDX12SpotLight(this, mesh, name, diffuseMap, colour, strength, position, rotation, scale);
		mObjManager->AddSpotLight(s);
		return s;
	}

	CDirectionalLight* CDX12Engine::CreateDirectionalLight(const std::string& mesh,
		const std::string& name,
		const std::string& diffuseMap,
		const CVector3& colour,
		const float& strength,
		CVector3           position,
		CVector3           rotation,
		float              scale)
	{
		auto d = new CDX12DirectionalLight(this, mesh, name, diffuseMap, colour, strength, position, rotation, scale);
		mObjManager->AddDirLight(d);
		return d;
	}

	CPointLight* CDX12Engine::CreatePointLight(const std::string& mesh,
		const std::string& name,
		const std::string& diffuseMap,
		const CVector3& colour,
		const float& strength,
		CVector3           position,
		CVector3           rotation,
		float              scale)
	{
		auto p = new CDX12PointLight(this, mesh, name, diffuseMap, colour, strength, position, rotation, scale);
		mObjManager->AddPointLight(p);
		return p;
	}

	void CDX12Engine::Flush()
	{
		const uint64_t fenceValueForSignal = Signal();
		WaitForFenceValue(mFence, fenceValueForSignal, mFenceEvent);
	}

	void CDX12Engine::Resize(UINT width,
		UINT height)
	{
		if (mWindow->GetWindowWidth() != width || mWindow->GetWindowHeight() != height)
		{
			// Don't allow 0 size swap chain back buffers.
			const UINT newWidth = std::max(1u, width);
			const UINT newHeight = std::max(1u, height);

			Flush();

			// Reset everything
			for (uint32_t i = 0; i < mNumFrames; ++i)
			{
				// Any references to the back buffers must be released
				// before the swap chain can be resized.
				mBackBuffers[i]->mResource = nullptr;

				mFrameFenceValues[i] = mFrameFenceValues[mCurrentBackBufferIndex];
			}

			DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
			ThrowIfFailed(mSwapChain->GetDesc(&swapChainDesc));

			ThrowIfFailed(mSwapChain->ResizeBuffers(mNumFrames, newWidth, newHeight, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

			mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

			for (uint32_t i = 0; i < mNumFrames; ++i)
			{
				ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mBackBuffers[i]->mResource)));

				mDevice->CreateRenderTargetView(mBackBuffers[i]->mResource.Get(),
					nullptr,
					mBackBuffers[i]->mRTVHandle->mCpu);

				mBackBuffers[i]->mCurrentResourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;
			}

			mWindow->SetWindowSize(newWidth, newHeight);

			mViewport.Width = static_cast<FLOAT>(newWidth);
			mViewport.Height = static_cast<FLOAT>(newHeight);

			Flush();
		}
	}

	void CDX12Engine::CreatePipelineStateObjects()
	{
		try
		{
			mPbrPso = std::make_unique<CDX12PBRPSO>(this);
			mSkyPso = std::make_unique<CDX12SkyPSO>(this);
			mDepthOnlyPso = std::make_unique<CDX12DepthOnlyPSO>(this, false);
			mDepthOnlyTangentPso = std::make_unique<CDX12DepthOnlyPSO>(this, true);
		}
		catch (const std::exception& e)
		{
			throw std::exception(e.what());
		}
	}

	void CDX12Engine::LoadDefaultShaders()
	{
		try
		{
			std::string absolutePath = std::filesystem::current_path().string() + "/Source/Shaders/";

			vs = std::make_unique<CDX12VertexShader>(this, absolutePath + "SimpleShader.hlsl");
			ps = std::make_unique<CDX12PixelShader>(this, absolutePath + "SimpleShader.hlsl");

			mDepthOnlyPixelShader = std::make_unique<CDX12PixelShader>(this, absolutePath + "DepthOnly_ps");
			mDepthOnlyNormalPixelShader = std::make_unique<CDX12PixelShader>(this, absolutePath + "DepthOnlyNormal_ps");
			mBasicTransformVertexShader = std::make_unique<CDX12VertexShader>(this, absolutePath + "BasicTransform_vs");
			mPbrVertexShader = std::make_unique<CDX12VertexShader>(this, absolutePath + "PBRNoNormals_vs");
			mPbrNormalVertexShader = std::make_unique<CDX12VertexShader>(this, absolutePath + "PBR_vs");
			mTintedTexturePixelShader = std::make_unique<CDX12PixelShader>(this, absolutePath + "TintedTexture_ps");


			if (!vs || !ps) throw std::runtime_error("Error loading default shaders");


			// I have disabled those because they are not used at the moment and they take a while to compile

			//mPbrPixelShader = std::make_unique<CDX12PixelShader>(this, absolutePath + "PBRNoNormals_ps");
			//mPbrNormalPixelShader = std::make_unique<CDX12PixelShader>(this, absolutePath + "PBR_ps");
			//mSkyPixelShader = std::make_unique<CDX12PixelShader>(this, absolutePath + "Sky_ps");
			//mSkyVertexShader = std::make_unique<CDX12VertexShader>(this, absolutePath + "Sky_vs");
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error(e.what());
		}
	}


	void CDX12Engine::CheckRayTracingSupport() const
	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};

		const auto hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5,
			&options5,
			sizeof(options5));

		if (hr != S_OK) throw std::runtime_error("Error");

		if (options5.RaytracingTier < D3D12_RAYTRACING_TIER_1_0) throw std::runtime_error(
			"RayTracing Not Supported");
	}

	void CDX12Engine::EnableDebugLayer() const
	{
#if defined(_DEBUG)

		// Always enable the debug layer before doing anything DX12 related
		// so all possible errors generated while creating DX12 objects
		// are caught by the debug layer.
		ComPtr<ID3D12Debug1> debugInterface;

		if (D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)) != S_OK)
		{
			throw std::runtime_error("Impossible to enable debug layer");
		}

		ComPtr<ID3D12Debug>  spDebugController0;
		ComPtr<ID3D12Debug1> spDebugController1;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&spDebugController0)));
		ThrowIfFailed(debugInterface->QueryInterface(IID_PPV_ARGS(&spDebugController1)));

		debugInterface->EnableDebugLayer();

#endif
	}

	static void ReportLiveObjects()
	{
		IDXGIDebug1* dxgiDebug;
		DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));
		dxgiDebug->EnableLeakTrackingForThread();
		//dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
		dxgiDebug->Release();
	}

	ComPtr<IDXGIAdapter4> GetAdapter(bool useWarp)
	{
		ComPtr<IDXGIFactory4> dxgiFactory;

		UINT createFactoryFlags = 0;

#if defined(_DEBUG)

		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

#endif

		if (FAILED(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory))))
		{
			throw std::runtime_error("Could not create debug adapter");
		}

		ComPtr<IDXGIAdapter1> dxgiAdapter1;
		ComPtr<IDXGIAdapter4> dxgiAdapter4;

		if (useWarp)
		{
			if (FAILED(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1))))
			{
				throw std::runtime_error("Could not get warp adapter");
			}

			if (FAILED(dxgiAdapter1.As(&dxgiAdapter4)))
			{
				throw std::runtime_error("Could not parse adapter");
			}
		}
		else
		{
			SIZE_T maxDedicatedVideoMemory = 0;

			for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND;
				++i)
			{
				DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
				dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

				// Check to see if the adapter can create a D3D12 device without actually 
				// creating it. The adapter with the largest dedicated video memory
				// is favored.

				if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 && SUCCEEDED(
					D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(
						ID3D12Device), nullptr)) && dxgiAdapterDesc1.DedicatedVideoMemory >
					maxDedicatedVideoMemory)
				{
					maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
					ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
				}
			}
		}
		return dxgiAdapter4;
	}

	ComPtr<ID3D12Device5> CreateDevice(ComPtr<IDXGIAdapter4> adapter)
	{
		ComPtr<ID3D12Device5> d3d12Device5;
		if (FAILED(
			D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device5)
			))) {
			throw std::runtime_error("Error Creating device");
		}

		// Enable debug messages in debug mode.

#if defined(_DEBUG)

		ComPtr<ID3D12InfoQueue> pInfoQueue;

		if (SUCCEEDED(d3d12Device5.As(&pInfoQueue)))
		{
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);

			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);

			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

			// Suppress whole categories of messages

			//D3D12_MESSAGE_CATEGORY Categories[] = {};

			// Suppress messages based on their severity level

			D3D12_MESSAGE_SEVERITY Severities[] = { D3D12_MESSAGE_SEVERITY_INFO };

			// Suppress individual messages by their ID

			D3D12_MESSAGE_ID DenyIds[] = {
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
				// I'm really not sure how to avoid this message.
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
				// This warning occurs when using capture frame while graphics debugging.
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
				// This warning occurs when using capture frame while graphics debugging.
			};

			D3D12_INFO_QUEUE_FILTER NewFilter = {};

			//NewFilter.DenyList.NumCategories = _countof(Categories);
			//NewFilter.DenyList.pCategoryList = Categories;

			NewFilter.DenyList.NumSeverities = _countof(Severities);
			NewFilter.DenyList.pSeverityList = Severities;
			NewFilter.DenyList.NumIDs = _countof(DenyIds);
			NewFilter.DenyList.pIDList = DenyIds;

			if (FAILED(pInfoQueue->PushStorageFilter(&NewFilter)))
			{
				throw std::runtime_error("Error");
			}
		}

#endif

		return d3d12Device5;
	}


	bool CheckTearingSupport()
	{
		BOOL allowTearing = FALSE;

		// Rather than create the DXGI 1.5 factory interface directly, we create the
		// DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
		// graphics debugging tools which will not support the 1.5 factory interface 
		// until a future update.
		ComPtr<IDXGIFactory4> factory4;
		if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
		{
			ComPtr<IDXGIFactory5> factory5;
			if (SUCCEEDED(factory4.As(&factory5)))
			{
				if (FAILED(
					factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &
						allowTearing, sizeof(allowTearing)))) {
					allowTearing = FALSE;
				}
			}
		}

		return allowTearing == TRUE;
	}


	uint64_t CDX12Engine::Signal()
	{
		const uint64_t fenceValueForSignal = ++mFenceValue;
		if (FAILED(mCommandQueue->Signal(mFence.Get(), fenceValueForSignal)))
		{
			throw std::runtime_error("Error");
		}

		return fenceValueForSignal;
	}

	// Wait for pending GPU work to complete.
	void CDX12Engine::WaitForGpu() noexcept
	{
		if (mCommandQueue.Get() && mFence.Get())
		{
			// Schedule a Signal command in the GPU queue.
			const UINT64 fenceValue = mFrameFenceValues[mCurrentBackBufferIndex];
			if (SUCCEEDED(mCommandQueue->Signal(mFence.Get(), fenceValue)))
			{
				// Wait until the Signal has been processed.
				if (SUCCEEDED(mFence->SetEventOnCompletion(fenceValue, mFenceEvent)))
				{
					std::ignore = WaitForSingleObjectEx(mFenceEvent, INFINITE, FALSE);

					// Increment the fence value for the current frame.
					mFrameFenceValues[mCurrentBackBufferIndex]++;
				}
			}
		}
	}


	HANDLE CreateEventHandle()
	{
		const HANDLE fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		assert(fenceEvent && "Failed to create fence event.");

		return fenceEvent;
	}


	void CDX12Engine::InitD3D()
	{

		auto sdk = LoadLibrary(L"D3d12SDKLayers.dll");

		// Create viewport
		mViewport = CD3DX12_VIEWPORT(0.0f,
			0.0f,
			static_cast<FLOAT>(mWindow->GetWindowWidth()),
			static_cast<FLOAT>(mWindow->GetWindowHeight()));

		mScissorRect = CD3DX12_RECT(0, 0, mWindow->GetWindowWidth(), mWindow->GetWindowHeight());

		// Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
		// Using this awareness context allows the client area of the window 
		// to achieve 100% scaling while still allowing non-client window content to 
		// be rendered in a DPI sensitive fashion.
		SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

		EnableDebugLayer();

		const ComPtr<IDXGIAdapter4> dxgiAdapter4 = GetAdapter(mUseWarp);

		mDevice = CreateDevice(dxgiAdapter4);

		NAME_D3D12_OBJECT(mDevice);


		// Create command queue
		{
			D3D12_COMMAND_QUEUE_DESC desc;
			desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			desc.NodeMask = 0;

			if (FAILED(mDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&mCommandQueue))))
			{
				throw std::runtime_error("Error creating command queue");
			}

			NAME_D3D12_OBJECT(mCommandQueue);
		}

		// Create swap chain
		{
			ComPtr<IDXGISwapChain4> dxgiSwapChain4;
			ComPtr<IDXGIFactory4>   dxgiFactory4;
			UINT                    createFactoryFlags;

#if defined(_DEBUG)
			createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#else
			createFactoryFlags = 0;
#endif

			if (FAILED(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4))))
			{
				throw std::runtime_error("Error create DXGIFactory");
			}

			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

			swapChainDesc.Width = mWindow->GetWindowWidth();
			swapChainDesc.Height = mWindow->GetWindowHeight();
			swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.SampleDesc = { 1,0 };
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = mNumFrames;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			// It is recommended to always allow tearing if tearing support is available.
			swapChainDesc.Flags = (CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0) | DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

			ComPtr<IDXGISwapChain1> swapChain1;
			if (FAILED(
				dxgiFactory4->CreateSwapChainForHwnd(mCommandQueue.Get(), mWindow->GetHandle(),
					&swapChainDesc, nullptr, nullptr, &swapChain1)))
			{
				throw std::runtime_error("Error creating swap chain");
			}

			// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
			// will be handled manually.
			ThrowIfFailed(dxgiFactory4->MakeWindowAssociation(mWindow->GetHandle(), DXGI_MWA_NO_ALT_ENTER));

			if (FAILED(swapChain1.As(&mSwapChain)))
			{
				throw std::runtime_error("Error casting swap chain");
			}

			mSwapChain->SetMaximumFrameLatency(3);

		}

		mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

		// Create descriptor heaps
		try
		{
			// Describe and create a render target view (RTV) descriptor heap.
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.NumDescriptors = 100;
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

			mRTVDescriptorHeap = std::make_unique<CDX12DescriptorHeap>(this, desc);

			NAME_D3D12_OBJECT(mRTVDescriptorHeap->mDescriptorHeap);

			//Describe and create a shader resource view (SRV) descriptor heap.

			desc.NumDescriptors = 200;
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

			mSRVDescriptorHeap = std::make_unique<CDX12DescriptorHeap>(this, desc);

			NAME_D3D12_OBJECT(mSRVDescriptorHeap->mDescriptorHeap);

			// Describe and create a sampler descriptor heap
			desc.NumDescriptors = 1;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;

			mSamplerDescriptorHeap = std::make_unique<CDX12DescriptorHeap>(this, desc);
			NAME_D3D12_OBJECT(mSamplerDescriptorHeap->mDescriptorHeap);
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error(e.what());
		}



		// Create command allocators
		{
			for (int i = 0; i < mNumFrames; ++i)
			{
				if (FAILED(
					mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS
					(&mCommandAllocators[i]))))
				{
					throw std::runtime_error("Error creating command allocator");
				}

				NAME_D3D12_OBJECT_INDEXED(mCommandAllocators, i);
			}

			for (size_t i = 0; i < ARRAYSIZE(mAmbientMapCommandAllocators); ++i)
			{
				auto commandAllocator = mAmbientMapCommandAllocators[i].GetAddressOf();
				ThrowIfFailed(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocator)))
					NAME_D3D12_OBJECT_INDEXED(mAmbientMapCommandAllocators, i);
			}
		}

		// Create command list 
		{
			ThrowIfFailed(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocators
				[mCurrentBackBufferIndex].Get(), nullptr, IID_PPV_ARGS(&mCommandList)));

			mCommandList->Close();

			mCurrRecordingCommandList = mCommandList.Get();


			for (size_t i = 0; i < ARRAYSIZE(mAmbientMapCommandLists); ++i)
			{
				auto j = i * mNumFrames;

				ThrowIfFailed(
					mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
						mAmbientMapCommandAllocators[i * mNumFrames].Get(),
						nullptr,
						IID_PPV_ARGS(&mAmbientMapCommandLists[i])));

				mAmbientMapCommandLists[i]->Close();

				NAME_D3D12_OBJECT_INDEXED(mAmbientMapCommandLists, i);

			}

			NAME_D3D12_OBJECT(mCommandList);
		}

		// Create fence
		{
			if (FAILED(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence))))
			{
				throw std::runtime_error("Error creating fence");
			}

			NAME_D3D12_OBJECT(mFence);

			mFenceEvent = CreateEventHandle();
		}

		// Create the constant buffers.
		try
		{
			for (auto i = 0; i < 3; ++i)
			{
				mPerFrameConstantBuffer[i] = std::make_unique<CDX12ConstantBuffer>(this, mSRVDescriptorHeap.get(), sizeof(PerFrameConstants));
				mPerFrameConstantBuffer[i]->Copy(mPerFrameConstants[i]);
				NAME_D3D12_OBJECT(mPerFrameConstantBuffer[i]->Resource());

				mPerFrameLightsConstantBuffer[i] = std::make_unique<CDX12ConstantBuffer>(this, mSRVDescriptorHeap.get(), sizeof(PerFrameLights));
				mPerFrameLightsConstantBuffer[i]->Copy(mPerFrameLights[i]);
				NAME_D3D12_OBJECT(mPerFrameLightsConstantBuffer[i]->Resource());

				mPerFrameSpotLightsConstantBuffer[i] = std::make_unique<CDX12ConstantBuffer>(this, mSRVDescriptorHeap.get(), sizeof(PerFrameSpotLights));
				mPerFrameSpotLightsConstantBuffer[i]->Copy(mPerFrameSpotLights[i]);
				NAME_D3D12_OBJECT(mPerFrameSpotLightsConstantBuffer[i]->Resource());

				mPerFrameDirLightsConstantBuffer[i] = std::make_unique<CDX12ConstantBuffer>(this, mSRVDescriptorHeap.get(), sizeof(PerFrameDirLights));
				mPerFrameDirLightsConstantBuffer[i]->Copy(mPerFrameDirLights[i]);
				NAME_D3D12_OBJECT(mPerFrameDirLightsConstantBuffer[i]->Resource());

				mPerFramePointLightsConstantBuffer[i] = std::make_unique<CDX12ConstantBuffer>(this, mSRVDescriptorHeap.get(), sizeof(PerFramePointLights));
				mPerFramePointLightsConstantBuffer[i]->Copy(mPerFramePointLights[i]);
				NAME_D3D12_OBJECT(mPerFramePointLightsConstantBuffer[i]->Resource());
			}
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error(e.what());
		}


		// Call the function ReportLiveObjects when the program exits
		atexit(&ReportLiveObjects);
	}


	void CDX12Engine::InitFrameDependentResources()
	{
		// Create frame resources
		{
			for (int i = 0; i < mNumFrames; ++i)
			{
				ComPtr<ID3D12Resource> res;

				if (FAILED(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&res))))
				{
					throw std::runtime_error("Error getting the current swap chain buffer");
				}

				mBackBuffers[i] = std::make_unique<CDX12RenderTarget>(this, res, mRTVDescriptorHeap.get());

				std::wstring s = L"BackBuffer" + i;

				mBackBuffers[i]->mResource->SetName(s.c_str());
			}
		}
	}


	void CDX12Engine::SetConstantBuffers()
	{
		mSRVDescriptorHeap->Set();

		mPerFrameConstantBuffer[mCurrentBackBufferIndex]->Set(1);
		mPerFrameLightsConstantBuffer[mCurrentBackBufferIndex]->Set(2);
		mPerFrameSpotLightsConstantBuffer[mCurrentBackBufferIndex]->Set(3);
		mPerFrameDirLightsConstantBuffer[mCurrentBackBufferIndex]->Set(4);
		mPerFramePointLightsConstantBuffer[mCurrentBackBufferIndex]->Set(5);
	}
}
