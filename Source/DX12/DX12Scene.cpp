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
#include "Objects/DX12SpotLight.h"

namespace DX12
{

	CDX12Scene::CDX12Scene(CDX12Engine* engine, std::string fileName) : CScene(engine, fileName)
	{
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

		// Render the ambient map
		auto m = MatrixIdentity();
		if (mAmbientMap) mAmbientMap->RenderFromThis(&m);

		// Set back the current recording command list
		mEngine->mCurrRecordingCommandList = commandList;

		// Render scene to texture
		{
			const FLOAT clearColor[] = { 0.4f,0.6f,0.9f,1.0f };

			const auto rtv = mSceneTexture->mRtvHeap->Get(mSceneTexture->mRTVHandle).mCpu;
			const auto dsv = mDSVDescriptorHeap->Get(mEngine->mCurrentBackBufferIndex).mCpu;

			mSceneTexture->Barrier(D3D12_RESOURCE_STATE_RENDER_TARGET);
			mDepthStencils[mEngine->mCurrentBackBufferIndex]->Barrier(D3D12_RESOURCE_STATE_DEPTH_WRITE);

			const auto vp = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<FLOAT>(mViewportX), static_cast<FLOAT>(mViewportY));
			const auto sr = CD3DX12_RECT(0, 0, mViewportX, mViewportY);

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

			mSceneTexture->Barrier(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			mDepthStencils[mEngine->mCurrentBackBufferIndex]->Barrier(D3D12_RESOURCE_STATE_DEPTH_READ);
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
		if (mAmbientMap->mEnable)
		{
			// Convert back from void* to handle*
			auto handle = mEngine->mSRVDescriptorHeap->Get(mAmbientMap->mSrvHandle).mGpu;
			mEngine->mCurrRecordingCommandList->SetGraphicsRootDescriptorTable(12, handle);
		}

		// Set the shadow maps
		for (const auto& l : mEngine->GetObjManager()->mSpotLights)
		{
			auto d = dynamic_cast<CDX12SpotLight*>(l);
			auto handle = mEngine->mSRVDescriptorHeap->Get(d->mSrvHandle).mGpu;
			mEngine->mCurrRecordingCommandList->SetGraphicsRootDescriptorTable(13, handle);
		}


		mEngine->GetObjManager()->RenderAllObjects();

		mShadowMaps.clear();
	}

	ImTextureID CDX12Scene::GetTextureSRV()
	{
		return reinterpret_cast<ImTextureID>(mSceneTexture->mSrvHeap->Get(mSceneTexture->mSrvHandle).mGpu.ptr);
	}

	void CDX12Scene::Resize(UINT newX, UINT newY)
	{
		mCamera->SetAspectRatio(float(newX) / float(newY));

		mViewportX = newX;
		mViewportY = newY;

		mEngine->FinalizeFrame();

		mEngine->Flush();

		mEngine->mCommandAllocators[mEngine->mCurrentBackBufferIndex]->Reset();
		mEngine->mCurrRecordingCommandList->Reset(mEngine->mCommandAllocators[mEngine->mCurrentBackBufferIndex].Get(), nullptr);

		for (auto& mDepthStencil : mDepthStencils)
		{
			mDepthStencil = nullptr;
		}

		mSceneTexture = nullptr;
		mDSVDescriptorHeap = nullptr;

		InitFrameDependentStuff();

		mEngine->CreateRTFrameDependentResources();

		mEngine->Flush();
	}

	CDX12Scene::~CDX12Scene()
	{
	}

	void CDX12Scene::UpdateScene(float& frameTime)
	{
		CScene::UpdateScene(frameTime);
	}

	void CDX12Scene::Save(std::string fileName) { CLevelImporter::SaveScene(fileName); }
	void CDX12Scene::PostProcessingPass() {  }
	void CDX12Scene::RenderToDepthMap() { }
	void CDX12Scene::DisplayPostProcessingEffects() {  }
}
