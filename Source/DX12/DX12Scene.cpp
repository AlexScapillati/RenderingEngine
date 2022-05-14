#include "DX12Scene.h"

#include "DX12AmbientMap.h"
#include "DX12DescriptorHeap.h"
#include "DX12Engine.h"
#include "DX12Texture.h"
#include "../Window.h"
#include "../../Common/Camera.h"
#include "../../Common/CGameObjectManager.h"
#include "../Common/LevelImporter.h"
#include "Objects/DX12Light.h"

#include "DXR/DXR.h"

namespace DX12
{

	CDX12Scene::CDX12Scene(CDX12Engine* engine, std::string fileName)
	{

		mEngine = engine;
		mWindow = mEngine->GetWindow();
		mFileName = fileName;
		mCamera = std::make_unique<CCamera>();

		if (fileName.empty())
		{
			//--------------------------------------------------------------------------------------
			// Scene Geometry and Layout
			//--------------------------------------------------------------------------------------

			mBackgroundColor = { 0.1f,0.1f,0.1f,1.0f };

			mCamera = std::make_unique<CCamera>();
			mCamera->SetPosition({ 0, 0, -40 });

			auto sky = mEngine->CreateSky("Stars.x", "sky", "lauter_waterfall.jpg");
			sky->SetScale(1000);

			//auto cube = mEngine->CreateObject("New Marquna Marble_vdinddyc_Surface", "Cube");
			//cube->SetPosition({ 20,10,0 });

			auto sphere = mEngine->CreateObject("Steel_sfcqbiec_Surface", "Sphere");

			//auto ground = mEngine->CreateObject("Ground", "Ground");


			////mEngine->CreateObject("Roman Statue_tfprbilda_3D Asset","statue",CVector3(10,20,0));

			////auto light = mEngine->CreateSpotLight("Light.x", "PointLight", "Flare.jpg", { 1,1,1 }, 1000);
			////light->SetPosition({ 20,10.f,10.f });
			////light->SetRotation({ ToRadians(180),0,0 });

			//auto l = mEngine->CreateLight("Light.x", "Light", "Flare.jpg", { 1,1,1 }, 10000);
			//l->SetPosition({ 10.f,50.f,30.f });

			//auto l1 = mEngine->CreateLight("Light.x", "Light", "Flare.jpg", { 1,1,1 }, 10000);
			//l1->SetPosition({ -10.f,-50.f,-30.f });

			//auto l3 = mEngine->CreateLight("Light.x", "Light", "Flare.jpg", { 1,1,1 }, 10000);
			//l3->SetPosition({ 0.f,0.f,0.f });

		}
		else
		{
			CLevelImporter::LoadScene(fileName, mEngine);
		}


		mEngine = engine;
		try
		{
			InitFrameDependentStuff();
			engine->InitRaytracing();

			mAmbientMap = std::make_unique<CDX12AmbientMap>(mEngine, 1024, mEngine->mSRVDescriptorHeap.get());
		}
		catch (const std::runtime_error& e) { throw std::runtime_error(e.what()); }
	}


	void CDX12Scene::InitFrameDependentStuff()
	{
		// Create descriptors
		{
			D3D12_DESCRIPTOR_HEAP_DESC desc{};
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			desc.NumDescriptors = 10;

			mDSVDescriptorHeap = std::make_unique<CDX12DescriptorHeap>(mEngine, desc);
			mDSVDescriptorHeap->mDescriptorHeap->SetName(L"DSVDescriptorHeap");
		}

		// Create scene texture
		{
			const CD3DX12_RESOURCE_DESC desc(
				D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				0,
				mViewportX,
				mViewportY,
				1,
				1,
				DXGI_FORMAT_R8G8B8A8_UNORM,
				1,
				0,
				D3D12_TEXTURE_LAYOUT_UNKNOWN,
				D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
			);

			mSceneTexture = std::make_unique <CDX12RenderTarget>(mEngine, desc, mEngine->mSRVDescriptorHeap.get(), mEngine->mRTVDescriptorHeap.get());

			mSceneTexture->mResource->SetName(L"SceneTex");
		}

		//Creating the depth texture
		{
			const CD3DX12_RESOURCE_DESC depth_texture(
				D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				0,
				mViewportX,
				mViewportY,
				1,
				1,
				DXGI_FORMAT_D32_FLOAT,
				1,
				0,
				D3D12_TEXTURE_LAYOUT_UNKNOWN,
				D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL |
				D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE
			);

			for (auto i = 0u; i < CDX12Engine::mNumFrames; i++)
			{
				mDepthStencils[i] = std::make_unique<CDX12DepthStencil>(mEngine, depth_texture, mEngine->mSRVDescriptorHeap.get(), mDSVDescriptorHeap.get());
				mDepthStencils[i]->mResource->SetName(L"DepthStencil");
			}
		}
		
	}


	void CDX12Scene::RenderScene(float& frameTime)
	{

		auto commandList = mEngine->GetCommandList();

		mEngine->mCurrRecordingCommandList = commandList;

		if(mEngine->mRaytracing)
		{
			auto prev = mSceneTexture->mCurrentResourceState;
			mSceneTexture->Barrier( D3D12_RESOURCE_STATE_COPY_DEST);

			commandList->CopyResource(mSceneTexture->mResource.Get(), mEngine->mOutputResource.Get());

			mSceneTexture->Barrier( prev);

			return;
		}

		// Render from lights
		{
			PIXBeginEvent(commandList, 0, L"Shadow maps Rendering");

			auto objm = mEngine->GetObjManager();

			for (const auto& l : objm->mSpotLights)
			{
				mShadowMaps.push_back(l->RenderFromThis());
			}

			for (const auto& l : objm->mPointLights)
			{
				mShadowMaps.push_back(l->RenderFromThis());
			}

			for (const auto& l : objm->mDirLights)
			{
				mShadowMaps.push_back(l->RenderFromThis());
			}

			PIXEndEvent(commandList);
		}

		//mEngine->SetPBRPSO();

		//// Set the shadow maps
		//if (!mShadowMaps.empty())
		//{
		//	if (void* ptr = mShadowMaps.front())
		//	{
		//		mEngine->mSRVDescriptorHeap->Set();
		//		// Convert back from void* to handle*
		//		auto handle = *static_cast<CD3DX12_GPU_DESCRIPTOR_HANDLE*>(ptr);
		//		mEngine->mCurrRecordingCommandList->SetGraphicsRootDescriptorTable(13, handle);
		//	}
		//}
		
		// Render the ambient map
		auto m = MatrixIdentity();
		if (mAmbientMap) mAmbientMap->RenderFromThis(&m);

		// Set back the current recording command list
		mEngine->mCurrRecordingCommandList = commandList;

		// Render scene to texture
		{
			const FLOAT clearColor[] = { 0.4f,0.6f,0.9f,1.0f };
			
			const auto rtv = mSceneTexture->mRTVHandle->mCpu;
			const auto dsv = mDSVDescriptorHeap->Get(mEngine->mCurrentBackBufferIndex)->mCpu;

			mSceneTexture->Barrier(D3D12_RESOURCE_STATE_RENDER_TARGET);
			mDepthStencils[mEngine->mCurrentBackBufferIndex]->Barrier(D3D12_RESOURCE_STATE_DEPTH_WRITE);

			const auto vp = CD3DX12_VIEWPORT(0.0f,0.0f,static_cast<FLOAT>(mViewportX),static_cast<FLOAT>(mViewportY));
			const auto sr = CD3DX12_RECT(0,0,mViewportX,mViewportY);

			{
				PIXBeginEvent(commandList, 0, "Rendering");

				// Set the viewport
				commandList->RSSetViewports(1, &vp);
				commandList->RSSetScissorRects(1, &sr);
				commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
				commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
				commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

				// Render the models from the camera pov
				RenderSceneFromCamera(mCamera.get());

				PIXEndEvent(commandList);
			}

			mSceneTexture->Barrier( D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			mDepthStencils[mEngine->mCurrentBackBufferIndex]->Barrier( D3D12_RESOURCE_STATE_DEPTH_READ);
		}
	}

	void CDX12Scene::RenderSceneFromCamera(CCamera* camera)
	{
		// Set camera matrices in the constant buffer and send over to GPU
		auto& perFrameConstants = mEngine->mPerFrameConstants[mEngine->mCurrentBackBufferIndex];
		perFrameConstants.cameraMatrix = camera->WorldMatrix();
		perFrameConstants.viewMatrix = camera->ViewMatrix();
		perFrameConstants.projectionMatrix = camera->ProjectionMatrix();
		perFrameConstants.viewProjectionMatrix = camera->ViewProjectionMatrix();
		perFrameConstants.ambientColour = gAmbientColour;
		perFrameConstants.parallaxMinSamples = 5;
		perFrameConstants.parallaxMaxSamples = 20;


		mEngine->UpdateLightsBuffers();
		mEngine->CopyBuffers();

		mEngine->SetSkyPSO();
		mEngine->mSRVDescriptorHeap->Set();
		mEngine->SetConstantBuffers();

		auto sky = mEngine->GetObjManager()->mSky;
		sky->Render();

		mEngine->SetPBRPSO();
		mEngine->mSRVDescriptorHeap->Set();
		mEngine->SetConstantBuffers();

		// Set ambient map
		// Convert back from void* to handle*

		if (mAmbientMap->mEnable)
		{
			auto handle = mEngine->mSRVDescriptorHeap->Get(mAmbientMap->mSrvHandle)->mGpu;
			mEngine->mCurrRecordingCommandList->SetGraphicsRootDescriptorTable(12, handle);
		}

		mEngine->GetObjManager()->RenderAllObjects();

		mShadowMaps.clear();
	}

	ImTextureID CDX12Scene::GetTextureSRV()
	{
		return reinterpret_cast<ImTextureID>(mSceneTexture->mSrvHandle->mGpu.ptr);
	}

	void CDX12Scene::Resize(UINT newX, UINT newY)
	{
		mCamera->SetAspectRatio(float(newX) / float(newY));

		return;

		mViewportX = newX;
		mViewportY = newY;
		

		mEngine->Flush();
		mEngine->WaitForGpu();

		mEngine->mCurrRecordingCommandList->Close();
		mEngine->mCommandAllocators[mEngine->mCurrentBackBufferIndex]->Reset();

		for (int i = 0; i < CDX12Engine::mNumFrames; ++i)
		{
			mDepthStencils[i] = nullptr;
		}

		mSceneTexture = nullptr;
		mDSVDescriptorHeap = nullptr;

		InitFrameDependentStuff();

		mEngine->Flush();

		mEngine->mCurrRecordingCommandList->Reset(mEngine->mCommandAllocators[mEngine->mCurrentBackBufferIndex].Get(),nullptr);
	}

	CDX12Scene::~CDX12Scene()
	{
	}

	// Not implemented

	void CDX12Scene::UpdateScene(float& frameTime)
	{
		// Toggle FPS limiting
		if (KeyHit(Key_P))
		{
			mLockFPS = !mLockFPS;
		}

		mCamera->Control(frameTime);


		// Show frame time / FPS in the window title //
		const auto   fpsUpdateTime = 0.5f; // How long between updates (in seconds)
		static float totalFrameTime = 0;
		static auto  frameCount = 0;
		totalFrameTime += frameTime;
		++frameCount;
		if (totalFrameTime > fpsUpdateTime)
		{
			// Displays FPS rounded to nearest int, and frame time (more useful for developers) in milliseconds to 2 decimal places
			const auto         avgFrameTime = totalFrameTime / frameCount;
			std::ostringstream frameTimeMs;
			frameTimeMs.precision(2);
			frameTimeMs << std::fixed << avgFrameTime * 1000;
			const auto windowTitle = "DirectX - Game Engine Test " + frameTimeMs.str() +
				"ms, CPU: " + std::to_string(static_cast<int>(1 / avgFrameTime + 0.5f));
			SetWindowTextA(mEngine->GetWindow()->GetHandle(), windowTitle.c_str());
			totalFrameTime = 0;
			frameCount = 0;
		}
	}
	void CDX12Scene::Save(std::string fileName) {
		CLevelImporter::SaveScene(fileName);
	}
}
