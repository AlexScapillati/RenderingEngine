#include "DX12Scene.h"

#include "DX12DescriptorHeap.h"
#include "DX12Texture.h"

#include "Objects/DX12Light.h"

#include "../Window.h"
#include "../../Common/LevelImporter.h"
#include "../../Common/Camera.h"

namespace DX12
{

	CDX12Scene::CDX12Scene(CDX12Engine* engine, std::string fileName): CScene(engine, fileName)
	{
		mEngine = engine;
		try
		{
			InitFrameDependentStuff();
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
			desc.NumDescriptors = 3;

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

			mSceneTexture = std::make_unique <CDX12RenderTarget>(mEngine, desc,mEngine->mSRVDescriptorHeap.get());

			NAME_D3D12_OBJECT(mSceneTexture->mResource);
		}

		//Creating the depth texture
		{
			const CD3DX12_RESOURCE_DESC depth_texture(
				D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				0,
				mWindow->GetWindowWidth(),
				mWindow->GetWindowHeight(),
				1,
				1,
				DXGI_FORMAT_D32_FLOAT,
				1,
				0,
				D3D12_TEXTURE_LAYOUT_UNKNOWN,
				D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL |
				D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE
				);

			for (int i = 0; i < CDX12Engine::mNumFrames; i++)
			{
				mDepthStencils[i] = std::make_unique<CDX12DepthStencil>(mEngine, depth_texture, mEngine->mSRVDescriptorHeap.get(), mDSVDescriptorHeap.get());
				mDepthStencils[i]->mResource->SetName(L"DepthStencil");
			}
		}
	}

	
	void CDX12Scene::RenderScene(float& frameTime)
	{
		const FLOAT clearColor[] = { 0.4f,0.6f,0.9f,1.0f };

		mEngine->mSRVDescriptorHeap->Set();

		const auto rtv = mSceneTexture->mRTVHandle.mCpu;

		const auto dsv = mDSVDescriptorHeap->Get(mEngine->mCurrentBackBufferIndex).mCpu;

		mSceneTexture->Barrier(D3D12_RESOURCE_STATE_RENDER_TARGET);
		mDepthStencils[mEngine->mCurrentBackBufferIndex]->Barrier(D3D12_RESOURCE_STATE_DEPTH_WRITE);

		const auto vp = CD3DX12_VIEWPORT(
			0.0f,
			0.0f,
			static_cast<FLOAT>(mViewportX),
			static_cast<FLOAT>(mViewportY));

		const auto sr = CD3DX12_RECT(
			0,
			0,
			mViewportX,
			mViewportY);

		// Set the viewport
		mEngine->mCommandList->RSSetViewports(1, &vp);
		mEngine->mCommandList->RSSetScissorRects(1, &sr);
		mEngine->mCommandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
		mEngine->mCommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
		mEngine->mCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		// Render the models from the camera pov
		RenderSceneFromCamera(mCamera.get());

		mSceneTexture->Barrier(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		mDepthStencils[mEngine->mCurrentBackBufferIndex]->Barrier(D3D12_RESOURCE_STATE_DEPTH_READ);
	}

	void CDX12Scene::RenderSceneFromCamera(CCamera* camera)
	{
		// Set camera matrices in the constant buffer and send over to GPU

		mEngine->mCBVDescriptorHeap->Set();

		auto& perFrameConstants                = mEngine->mPerFrameConstants;
		perFrameConstants.cameraMatrix         = camera->WorldMatrix();
		perFrameConstants.viewMatrix           = camera->ViewMatrix();
		perFrameConstants.projectionMatrix     = camera->ProjectionMatrix();
		perFrameConstants.viewProjectionMatrix = camera->ViewProjectionMatrix();
		perFrameConstants.ambientColour        = gAmbientColour;

		mEngine->UpdateLightsBuffers();

		mEngine->CopyBuffers();

		PIXBeginEvent(mEngine->mCommandList.Get(), 0, "Rendering");

		mEngine->GetObjManager()->RenderAllObjects();

		PIXEndEvent(mEngine->mCommandList.Get());
	}

	ImTextureID CDX12Scene::GetTextureSRV()
	{
		return reinterpret_cast<ImTextureID>(mSceneTexture->mHandle.mGpu.ptr);
	}
	
	void CDX12Scene::Resize(UINT newX, UINT newY)
	{
		return;

		mEngine->Flush();

		ThrowIfFailed(mEngine->mCommandList->Close());

		mCamera->SetAspectRatio(float(newX) / float(newY));

		mViewportX = newX;
		mViewportY = newY;


		for (int i = 0; i < CDX12Engine::mNumFrames; ++i)
		{
			mEngine->mDepthStencils[i] = nullptr;
		}

		mSceneTexture = nullptr;
		mDSVDescriptorHeap = nullptr;

		InitFrameDependentStuff();

		// Reset the current command allocator and the command list
		mEngine->mCommandAllocators[mEngine->mCurrentBackBufferIndex]->Reset();

		ThrowIfFailed(mEngine->mCommandList->Reset(mEngine->mCommandAllocators[mEngine->mCurrentBackBufferIndex].Get(), nullptr));

		mEngine->mBackBuffers[mEngine->mCurrentBackBufferIndex]->mCurrentResourceState = D3D12_RESOURCE_STATE_COMMON;
	}

	CDX12Scene::~CDX12Scene()
	{
	}

	void CDX12Scene::UpdateScene(float& frameTime) { CScene::UpdateScene(frameTime); }
	void CDX12Scene::Save(std::string fileName) { CScene::Save(fileName); }
	void CDX12Scene::PostProcessingPass() {  }
	void CDX12Scene::RenderToDepthMap() { }
	void CDX12Scene::DisplayPostProcessingEffects() {  }
}
