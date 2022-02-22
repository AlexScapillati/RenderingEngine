#include "DX12Scene.h"

#include "DX12DescriptorHeap.h"
#include "DX12Texture.h"

#include "Objects/DX12Light.h"

#include "../Window.h"
#include "../../Common/LevelImporter.h"
#include "../../Common/Camera.h"

namespace DX12
{
	CDX12Scene::CDX12Scene(CDX12Engine* engine): CScene(engine)
	{
		mEngine = engine;

		mFileName = "";

		try
		{
			InitFrameDependentStuff();
		}
		catch (const std::runtime_error& e) { throw std::runtime_error(e.what()); }
	}

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

			mSceneTexture = std::make_unique <CDX12RenderTarget>(mEngine, desc);

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

			D3D12_CLEAR_VALUE clear_value;
			clear_value.Format               = DXGI_FORMAT_D32_FLOAT;
			clear_value.DepthStencil.Depth   = 1.0f;
			clear_value.DepthStencil.Stencil = 0;


			const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			for (int i = 0; i < CDX12Engine::mNumFrames; i++)
			{
				const auto result = mEngine->mDevice->CreateCommittedResource(
					&heapProperties,
					D3D12_HEAP_FLAG_NONE,
					&depth_texture,
					D3D12_RESOURCE_STATE_DEPTH_WRITE,
					&clear_value,
					IID_PPV_ARGS(&mEngine->mDepthStencils[i])
					);

				NAME_D3D12_OBJECT_INDEXED(mEngine->mDepthStencils, i);

				assert(result == S_OK && "CREATING THE DEPTH STENCIL FAILED");

				const auto dsv = mEngine->mDSVDescriptorHeap->Add().mCpu;

				mEngine->mDevice->CreateDepthStencilView(mEngine->mDepthStencils[i].Get(), nullptr, dsv);

				mEngine->mDevice->Release();
			}
		}
	}

	
	void CDX12Scene::RenderScene(float& frameTime)
	{
		const FLOAT clearColor[] = { 0.4f,0.6f,0.9f,1.0f };

		const auto rtv = mSceneTexture->mRTVHandle.mCpu;

		const auto dsv = mEngine->mDSVDescriptorHeap->Get(mEngine->mCurrentBackBufferIndex).mCpu;

		mSceneTexture->Barrier(D3D12_RESOURCE_STATE_RENDER_TARGET);

		const auto vp = CD3DX12_VIEWPORT(
			0.0f,
			0.0f,
			static_cast<FLOAT>(mViewportX),
			static_cast<FLOAT>(mViewportY));

		const auto sr = CD3DX12_RECT(
			0,
			0,
			(mViewportX),
			(mViewportY));

		// Set the viewport
		mEngine->mCommandList->RSSetViewports(1, &vp);
		mEngine->mCommandList->RSSetScissorRects(1, &sr);
		mEngine->mCommandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
		mEngine->mCommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
		mEngine->mCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		// Render the models from the camera pov
		RenderSceneFromCamera(mCamera.get());

		mSceneTexture->Barrier(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
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
		perFrameConstants.ambient              = gAmbientColour;

		// todo: mGameobjectManager->updateLightsbuffers

		if (!mObjManager->mLights.empty())
		{
			const auto   light                 = dynamic_cast<CDX12Light*>(mObjManager->mLights.front());
			const SLight lightInfo             = { light->Position(),static_cast<float>(*light->Enabled()),light->GetColour(), light->GetStrength() };
			mEngine->mPerFrameLights.lights[0] = lightInfo;
		}

		mEngine->CopyBuffers();

		PIXBeginEvent(mEngine->mCommandList.Get(), 0, "Rendering");

		mObjManager->RenderAllObjects();

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

		mSceneTexture->Barrier(D3D12_RESOURCE_STATE_COMMON);

		ThrowIfFailed(mEngine->mCommandList->Close());

		mCamera->SetAspectRatio(float(newX) / float(newY));

		mViewportX = newX;
		mViewportY = newY;

		mSceneTexture = nullptr;

		for (int i = 0; i < CDX12Engine::mNumFrames; ++i)
		{
			mEngine->mDepthStencils[i] = nullptr;
		}

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
	void CDX12Scene::PostProcessingPass() { CScene::PostProcessingPass(); }
	void CDX12Scene::RenderToDepthMap() { CScene::RenderToDepthMap(); }
	void CDX12Scene::DisplayPostProcessingEffects() { CScene::DisplayPostProcessingEffects(); }
}
