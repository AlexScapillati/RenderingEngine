#include "DX12Scene.h"

<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
#include "DX12AmbientMap.h"
#include "DX12Engine.h"
=======
>>>>>>> parent of 5f2c2d1 (Working on IBL - DX12)
=======
=======
<<<<<<< Updated upstream
=======
#include "DX12AmbientMap.h"
>>>>>>> Stashed changes
#include "DX12DescriptorHeap.h"
>>>>>>> parent of 7bb1619 (Merge branch 'main' into TryingPolymorphism)
#include "DX12Engine.h"

#include "DX12AmbientMap.h"
>>>>>>> parent of 78525fa (Merge pull request #2 from AlexScapillati/TryingPolymorphism)
#include "DX12DescriptorHeap.h"
=======
=======
>>>>>>> parent of 5f2c2d1 (Working on IBL - DX12)
#include "DX12DescriptorHeap.h"
>>>>>>> parent of e03ed59 (Static Polymorphic attemp)
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


<<<<<<< HEAD
<<<<<<< HEAD
	void CDX12Scene::RenderScene(float& frameTime)
=======
<<<<<<< Updated upstream
			const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			for (int i = 0; i < CDX12Engine::mNumFrames; i++)
=======
	void CDX12Scene::RenderSceneImpl(float& frameTime)
>>>>>>> parent of 7bb1619 (Merge branch 'main' into TryingPolymorphism)
	{
		// Render from lights
		{
			PIXBeginEvent(mEngine->mCommandList.Get(), 0, L"Shadow maps Rendering");

			auto objm = mEngine->GetObjManager();

			for (const auto& l : objm->mSpotLights)
>>>>>>> Stashed changes
=======
			const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			for (int i = 0; i < CDX12Engine::mNumFrames; i++)
>>>>>>> parent of e03ed59 (Static Polymorphic attemp)
			{
<<<<<<< HEAD
				const auto result = mEngine->mDevice->CreateCommittedResource(
					&heapProperties,
					D3D12_HEAP_FLAG_NONE,
					&depth_texture,
					D3D12_RESOURCE_STATE_DEPTH_WRITE,
					&clear_value,
					IID_PPV_ARGS(&mEngine->mDepthStencils[i])
					);

<<<<<<< HEAD
=======
				mShadowMaps.push_back(l->RenderFromThis());
			}

>>>>>>> parent of 5f2c2d1 (Working on IBL - DX12)
			PIXEndEvent(mEngine->mCommandList.Get());
		}

		// Render scene to texture
		{
			const FLOAT clearColor[] = { 0.4f,0.6f,0.9f,1.0f };

			mEngine->mSRVDescriptorHeap->Set();

			const auto rtv = mSceneTexture->mRTVHandle.mCpu;

			const auto dsv = mDSVDescriptorHeap->Get(mEngine->mCurrentBackBufferIndex).mCpu;

			mSceneTexture->Barrier(D3D12_RESOURCE_STATE_RENDER_TARGET);
			mDepthStencils[mEngine->mCurrentBackBufferIndex]->Barrier(D3D12_RESOURCE_STATE_DEPTH_WRITE);
=======
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
>>>>>>> parent of 7bb1619 (Merge branch 'main' into TryingPolymorphism)

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

<<<<<<< HEAD
		mEngine->mPbrPso->Set(mEngine->mCommandList.Get());

		mEngine->mPbrPso->Set(mEngine->mCommandList.Get());

		mEngine->mSRVDescriptorHeap->Set();

		if (!mShadowMaps.empty())
		{
			void* ptr = mShadowMaps.front();
			// Convert back from void* to handle*
			auto handle = *static_cast<CD3DX12_GPU_DESCRIPTOR_HANDLE*>(ptr);
			//mEngine->mCommandList->SetGraphicsRootDescriptorTable(13, handle);
<<<<<<< HEAD
=======
		if (!mObjManager->mLights.empty())
		{
			const auto   light                 = mObjManager->mLights.front();
			const SLight lightInfo             = { light->Position(),static_cast<float>(*light->Enabled()),light->GetColour(), light->GetStrength() };
			mEngine->mPerFrameLights.lights[0] = lightInfo;
>>>>>>> parent of 7bb1619 (Merge branch 'main' into TryingPolymorphism)
=======
>>>>>>> parent of 5f2c2d1 (Working on IBL - DX12)
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
<<<<<<< HEAD
<<<<<<< HEAD

	void CDX12Scene::Resize(UINT newX, UINT newY)
=======
<<<<<<< Updated upstream
	
	void CDX12Scene::Resize(UINT newX, UINT newY)
=======

	void CDX12Scene::ResizeImpl(UINT newX, UINT newY)
>>>>>>> Stashed changes
>>>>>>> parent of 7bb1619 (Merge branch 'main' into TryingPolymorphism)
=======
	
	void CDX12Scene::Resize(UINT newX, UINT newY)
>>>>>>> parent of e03ed59 (Static Polymorphic attemp)
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
	void CDX12Scene::PostProcessingPass() {  }
	void CDX12Scene::RenderToDepthMap() { }
	void CDX12Scene::DisplayPostProcessingEffects() {  }
}
