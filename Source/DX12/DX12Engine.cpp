#include "DX12Engine.h"


#include <d3d12.h>

#include "d3dx12.h"

#include "../Utility/Input.h"
#include "..\Window.h"

#include "DX12Scene.h"
#include "DX12Gui.h"

#include "D3D12Init.h"

#include "D3D12Helpers.h"

CDX12Engine::CDX12Engine(HINSTANCE hInstance, int nCmdShow)
{
	// Prepare TL-Engine style input functions
	InitInput();

	//get the executable path
	CHAR path[MAX_PATH];

	GetModuleFileNameA(hInstance, path, MAX_PATH);

	const auto pos = std::string(path).find_last_of("\\/");

	//get the media folder
	mMediaFolder = std::string(path).substr(0, pos) + "\\Media\\";

	mMediaFolder = ReplaceAll(mMediaFolder, std::string("\\"), std::string("/"));

	try
	{
		// Create a window 
		mWindow = std::make_unique<CWindow>(hInstance, nCmdShow);

		// Initialise Direct3D
		InitD3D();

		InitFrameDependentResources();

		// Load Shaders
		LoadDefaultShaders();

		// Create Gui
		mGui = std::make_unique<CDX12Gui>(this);

		// Create scene
		mMainScene = std::make_unique<CDX12Scene>(this, "Scene1.xml");

	}
	catch (const std::runtime_error& e) { throw std::runtime_error(e.what()); }

	// Will use a timer class to help in this tutorial (not part of DirectX). It's like a stopwatch - start it counting now
	mTimer.Start();
}

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

			try
			{
				InitializeFrame();

				mGui->Begin(frameTime);

				mMainScene->UpdateScene(frameTime);

				// Draw the scene
				mMainScene->RenderScene(frameTime);

				//mGui
				//mGui->Show(frameTime);

				mGui->End();

				FinalizeFrame();
			}
			catch (const std::exception& e) { throw std::runtime_error(e.what()); }


			if (KeyHit(Key_Escape))
			{
				// Save automatically
				//mMainScene->Save();

				WaitForFenceValue(mFence, mFrameFenceValues[mCurrentBackBufferIndex], mFenceEvent);

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


void CDX12Engine::InitializeFrame()
{
	mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

	// Reset the current command allocator and the command list
	mCommandAllocators[mCurrentBackBufferIndex]->Reset();

	mCommandList->Reset(mCommandAllocators[mCurrentBackBufferIndex].Get(), nullptr);

	ID3D12DescriptorHeap* ppHeaps[] = { mSRVDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// Set the viewport
	mCommandList->RSSetViewports(1, &mViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Set and clear the render target.
	Barrier(D3D12_RESOURCE_STATE_RENDER_TARGET);

	const FLOAT clearColor[] = { 0.4f,0.6f,0.9f,1.0f };

	const CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(
		mRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		mCurrentBackBufferIndex, mRTVSize);

	const CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(
		mDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		mCurrentBackBufferIndex, mDSVSize);

	// TODO: remove mCurrentBackBufferIndex or make 3 depth stencils

	mCommandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
	mCommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
	mCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void CDX12Engine::CopyBuffers() const { memcpy(mFrameCbvDataBegin, &mPerFrameConstants, sizeof(mPerFrameConstants)); }


uint64_t CDX12Engine::ExecuteCommandList(ID3D12GraphicsCommandList2* commandList)
{
	commandList->Close();

	ID3D12CommandAllocator* commandAllocator;
	UINT                    dataSize = sizeof(commandAllocator);
	commandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator);

	ID3D12CommandList* const ppCommandLists[] = {
		commandList
	};

	mCommandQueue->ExecuteCommandLists(1, ppCommandLists);
	const uint64_t fenceValue = Signal();

	// The ownership of the command allocator has been transferred to the ComPtr
	// in the command allocator queue. It is safe to release the reference 
	// in this temporary COM pointer here.
	commandAllocator->Release();

	return fenceValue;
}

void CDX12Engine::WaitForFenceValue(
	ComPtr<ID3D12Fence>       fence, uint64_t fenceValue, HANDLE fenceEvent,
	std::chrono::milliseconds duration)
{
	if (fence->GetCompletedValue() < fenceValue)
	{
		if (FAILED(fence->SetEventOnCompletion(fenceValue, fenceEvent)))
		{
			throw std::runtime_error("Error waiting for the fence");
		}
		::WaitForSingleObject(fenceEvent, static_cast<DWORD>(duration.count()));
	}
}

void CDX12Engine::Flush()
{
	const uint64_t fenceValueForSignal = Signal();
	WaitForFenceValue(mFence, fenceValueForSignal, mFenceEvent);
}

void CDX12Engine::Resize(UINT width, UINT height)
{
	if (mWindow->GetWindowWidth() != width || mWindow->GetWindowHeight() != height)
	{
		// Don't allow 0 size swap chain back buffers.
		const UINT newWidth = std::max(1u, width);
		const UINT newHeight = std::max(1u, height);

		// Flush the GPU queue to make sure the swap chain's back buffers
		// are not being referenced by an in-flight command list.
		Flush();

		for (int i = 0; i < mNumFrames; ++i)
		{
			// Any references to the back buffers must be released
			// before the swap chain can be resized.
			mBackBuffers[i].Reset();
			mFrameFenceValues[i] = mFrameFenceValues[mCurrentBackBufferIndex];
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		ThrowIfFailed(mSwapChain->GetDesc(&swapChainDesc));

		ThrowIfFailed(mSwapChain->ResizeBuffers(mNumFrames, newWidth, newHeight,
			swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

		mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

		mWindow->SetWindowSize(newWidth, newHeight);
		mMainScene->Resize(newWidth, newHeight);

		InitFrameDependentResources();
	}
}

void CDX12Engine::FinalizeFrame()
{
	// Present
	{
		Barrier(D3D12_RESOURCE_STATE_COMMON);

		mCommandList->Close();

		ID3D12CommandList* const commandLists[] = {
			mCommandList.Get()
		};
		mCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

		mFrameFenceValues[mCurrentBackBufferIndex] = Signal();

		const UINT syncInterval = mMainScene->GetLockFps() ? 1 : 0;
		const UINT presentFlags = !mMainScene->GetLockFps() ? DXGI_PRESENT_ALLOW_TEARING : 0;
		if (FAILED(mSwapChain->Present(syncInterval, presentFlags))) { throw std::runtime_error("Error presenting"); }

		mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

		WaitForFenceValue(mFence, mFrameFenceValues[mCurrentBackBufferIndex], mFenceEvent);
	}
}

auto CDX12Engine::GetSceneTex() const
{
	return mMainScene->GetTextureSRV();
}

void CDX12Engine::Barrier(D3D12_RESOURCE_STATES after)
{
	const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		mBackBuffers[mCurrentBackBufferIndex].Get(),
		mCurrentResourceState,
		after);

	mCurrentResourceState = after;

	mCommandList->ResourceBarrier(1, &barrier);
}

void CDX12Engine::LoadDefaultShaders()
{
	try
	{
		mDepthOnlyPixelShader.LoadShaderFromFile("Source/Shaders/DepthOnly_ps");
		mDepthOnlyNormalPixelShader.LoadShaderFromFile(("Source/Shaders/DepthOnlyNormal_ps"));
		mBasicTransformVertexShader.LoadShaderFromFile(("Source/Shaders/BasicTransform_vs"));
		mPbrVertexShader.LoadShaderFromFile(("Source/Shaders/PBRNoNormals_vs"));
		mPbrNormalVertexShader.LoadShaderFromFile(("Source/Shaders/PBR_vs"));
		mPbrPixelShader.LoadShaderFromFile(("Source/Shaders/PBRNoNormals_ps"));
		mPbrNormalPixelShader.LoadShaderFromFile(("Source/Shaders/PBR_ps"));
		mTintedTexturePixelShader.LoadShaderFromFile(("Source/Shaders/TintedTexture_ps"));
		mSkyPixelShader.LoadShaderFromFile(("Source/Shaders/Sky_ps"));
		mSkyVertexShader.LoadShaderFromFile(("Source/Shaders/Sky_vs"));
	}
	catch (const std::exception& e) { throw std::runtime_error(e.what()); }
}
