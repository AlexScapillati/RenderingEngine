//--------------------------------------------------------------------------------------
// Scene geometry and layout preparation
// Scene rendering & update
//--------------------------------------------------------------------------------------

#include "DX11Scene.h"

#include <deque>
#include <deque>
#include <stdexcept>
#include <stdexcept>

#include "DX11Common.h"
#include "../Common/CGameObjectManager.h"
#include "..\Common/Camera.h"
#include "Objects/DX11DirLight.h"
#include "Objects/DX11Plant.h"
#include "Objects/DX11SpotLight.h"

namespace DX11
{
	//--------------------------------------------------------------------------------------
	// Constant Buffers
	//--------------------------------------------------------------------------------------
	// Variables sent over to the GPU each frame
	// The structures are now in Common.h
	// IMPORTANT: Any new data you add in C++ code (CPU-side) is not automatically available to the GPU
	//            Anything the shaders need (per-frame or per-model) needs to be sent via a constant buffer

	PerFrameConstants gPerFrameConstants;
	// The constants that need to be sent to the GPU each frame (see common.h for structure)
	ComPtr<ID3D11Buffer> gPerFrameConstantBuffer; // The GPU buffer that will recieve the constants above

	PerModelConstants    gPerModelConstants;      // As above, but constant that change per-model (e.g. world matrix)
	ComPtr<ID3D11Buffer> gPerModelConstantBuffer; // --"--

	PerFrameLights       gPerFrameLightsConstants;
	ComPtr<ID3D11Buffer> gPerFrameLightsConstBuffer;

	PerFrameSpotLights   gPerFrameSpotLightsConstants;
	ComPtr<ID3D11Buffer> gPerFrameSpotLightsConstBuffer;

	PerFrameDirLights    gPerFrameDirLightsConstants;
	ComPtr<ID3D11Buffer> gPerFrameDirLightsConstBuffer;

	PerFramePointLights  gPerFramePointLightsConstants;
	ComPtr<ID3D11Buffer> gPerFramePointLightsConstBuffer;

	PostProcessingConstants gPostProcessingConstants;
	ComPtr<ID3D11Buffer>    gPostProcessingConstBuffer;

	CDX11Scene::CDX11Scene(CDX11Engine* engine, std::string fileName) : CScene<CDX11Scene>(engine,fileName)
		{
			mEngine = engine;

			//--------------------------------------------------------------------------------------
			// Initialise scene DX11 stuff
			//--------------------------------------------------------------------------------------

			try
			{
				//create all the textures needed for the scene rendering
				InitTextures();
			}
			catch (const std::runtime_error& e) { throw std::runtime_error(e.what()); }


			////--------------- Load meshes ---------------////

			// Load mesh geometry data

			try { LoadPostProcessingImages(); }
			catch (const std::exception& e) { throw std::runtime_error(e.what()); }

			////--------------- GPU states ---------------////

			// Create all filtering modes, blending modes etc. used by the app (see State.cpp/.h)
			if (!mEngine->CreateStates()) { throw std::runtime_error("Error creating DirectX states"); }

			// Create GPU-side constant buffers to receive the gPerFrameConstants and gPerModelConstants structures above
			// These allow us to pass data from CPU to shaders such as lighting information or matrices
			// See the comments above where these variable are declared and also the UpdateScene function
			gPerFrameConstantBuffer.Attach(mEngine->CreateConstantBuffer(sizeof(gPerFrameConstants)));
			gPerModelConstantBuffer.Attach(mEngine->CreateConstantBuffer(sizeof(gPerModelConstants)));
			gPerFrameLightsConstBuffer.Attach(mEngine->CreateConstantBuffer(sizeof(gPerFrameLightsConstants)));
			gPerFrameSpotLightsConstBuffer.Attach(mEngine->CreateConstantBuffer(sizeof(gPerFrameSpotLightsConstants)));
			gPerFrameDirLightsConstBuffer.Attach(mEngine->CreateConstantBuffer(sizeof(gPerFrameDirLightsConstants)));
			gPerFramePointLightsConstBuffer.Attach(mEngine->CreateConstantBuffer(sizeof(gPerFramePointLightsConstants)));
			gPostProcessingConstBuffer.Attach(mEngine->CreateConstantBuffer(sizeof(gPostProcessingConstants)));

			if (!gPerFrameConstantBuffer || !gPerModelConstantBuffer || !gPerFrameDirLightsConstBuffer || !
				gPerFrameLightsConstBuffer || !gPerFrameSpotLightsConstBuffer || !gPerFramePointLightsConstBuffer || !
				gPostProcessingConstBuffer) { throw std::runtime_error("Error creating constant buffers"); }
		}

	
	//--------------------------------------------------------------------------------------
	// Scene Rendering
	//--------------------------------------------------------------------------------------

	// Render everything in the scene from the given camera
	void CDX11Scene::RenderSceneFromCameraImpl(CCamera* camera)
	{
		// Set camera matrices in the constant buffer and send over to GPU
		gPerFrameConstants.cameraMatrix         = camera->WorldMatrix();
		gPerFrameConstants.viewMatrix           = camera->ViewMatrix();
		gPerFrameConstants.projectionMatrix     = camera->ProjectionMatrix();
		gPerFrameConstants.viewProjectionMatrix = camera->ViewProjectionMatrix();

		// Update the frame constant buffer
		mEngine->UpdateFrameConstantBuffer(gPerFrameConstantBuffer.Get(), gPerFrameConstants);

		// Set it to the GPU
		mEngine->GetContext()->PSSetConstantBuffers(1, 1, gPerFrameConstantBuffer.GetAddressOf());
		mEngine->GetContext()->VSSetConstantBuffers(1, 1, gPerFrameConstantBuffer.GetAddressOf());

		////--------------- Render ordinary models ---------------///

		// Select which shaders to use next
		mEngine->GetContext()->GSSetShader(nullptr, nullptr, 0);
		////// Switch off geometry shader when not using it (pass nullptr for first parameter)

		// States - no blending, normal depth buffer and back-face culling (standard set-up for opaque models)
		mEngine->GetContext()->OMSetBlendState(mEngine->mNoBlendingState.Get(), nullptr, 0xffffff);
		mEngine->GetContext()->OMSetDepthStencilState(mEngine->mUseDepthBufferState.Get(), 0);
		mEngine->GetContext()->RSSetState(mEngine->mCullBackState.Get());

		mEngine->GetContext()->PSSetSamplers(0, 1, mEngine->mAnisotropic4XSampler.GetAddressOf());

		//Render All Objects, if something went wrong throw an exception
		mEngine->GetObjManager()->RenderAllObjects();

		mShadowsMaps.clear();

		// Unbind shadow maps from shaders - prevents warnings from DirectX when we try to render to the shadow maps again next frame
		const auto nShadowMaps = mEngine->GetObjManager()->mSpotLights.size() + mEngine->GetObjManager()->mDirLights.size() + (
			mEngine->GetObjManager()->mPointLights.size() * 6);

		for (int i = 0; i < nShadowMaps; ++i)
		{
			ID3D11ShaderResourceView* nullView = nullptr;
			mEngine->GetContext()->PSSetShaderResources(7 + i, 1, &nullView);
		}
	}


	void UpdateAllBuffers(CGameObjectManager* g);


	// Rendering the scene
	void CDX11Scene::RenderSceneImpl(float& frameTime)
	{
		//// Common settings ////

		// Set up the light information in the constant buffer
		// Don't send to the GPU yet, the function RenderSceneFromCamera will do that

		UpdateAllBuffers(mEngine->GetObjManager());

		gPerFrameConstants.ambientColour = gAmbientColour;
		gPerFrameConstants.specularPower = gSpecularPower;
		gPerFrameConstants.cameraPosition = mCamera->Position();
		gPerFrameConstants.frameTime = frameTime;
		gPerFrameConstants.nPcfSamples = mPcfSamples;

		// Update constant buffers
		mEngine->UpdateLightConstantBuffer(gPerFrameLightsConstBuffer.Get(),
			gPerFrameLightsConstants,
			static_cast<int>(mEngine->GetObjManager()->mLights.size()));
		mEngine->UpdateDirLightsConstantBuffer(gPerFrameDirLightsConstBuffer.Get(),
			gPerFrameDirLightsConstants,
			static_cast<int>(mEngine->GetObjManager()->mDirLights.size()));
		mEngine->UpdateSpotLightsConstantBuffer(gPerFrameSpotLightsConstBuffer.Get(),
			gPerFrameSpotLightsConstants,
			static_cast<int>(mEngine->GetObjManager()->mSpotLights.size()));
		mEngine->UpdatePointLightsConstantBuffer(gPerFramePointLightsConstBuffer.Get(),
			gPerFramePointLightsConstants,
			static_cast<int>(mEngine->GetObjManager()->mPointLights.size()));

		// Set them to the GPU
		ID3D11Buffer* frameCBuffers[] =
		{
			gPerFrameLightsConstBuffer.Get(),
			gPerFrameSpotLightsConstBuffer.Get(),
			gPerFrameDirLightsConstBuffer.Get(),
			gPerFramePointLightsConstBuffer.Get()
		};

		mEngine->GetContext()->PSSetConstantBuffers(2, 4, frameCBuffers);

		mEngine->GetContext()->VSSetConstantBuffers(2, 4, frameCBuffers);

		// Set the sampler for the material textures
		mEngine->GetContext()->PSSetSamplers(0, 1, mEngine->mAnisotropic4XSampler.GetAddressOf());

		// Set Sampler for the Shadow Maps
		mEngine->GetContext()->PSSetSamplers(1, 1, mEngine->mPointSamplerBorder.GetAddressOf());

		////----- Render form the lights point of view ----------////

		for (const auto it : mEngine->GetObjManager()->mSpotLights)
		{
			if (*it->Enabled())
			{
				auto tmp = static_cast<ID3D11ShaderResourceView*>(it->RenderFromThis());

				mShadowsMaps.push_back(tmp);
			}
		}

		for (const auto it : mEngine->GetObjManager()->mDirLights)
		{
			if (*it->Enabled())
			{
				auto tmp = static_cast<ID3D11ShaderResourceView*>(it->RenderFromThis());

				mShadowsMaps.push_back(tmp);
			}
		}

		for (const auto it : mEngine->GetObjManager()->mPointLights)
		{
			if (*it->Enabled())
			{
				auto tmp = static_cast<ID3D11ShaderResourceView**>(it->RenderFromThis());

				for (int i = 0; i < 6; ++i)
				{
					mShadowsMaps.push_back(tmp[i]);
				}
			}
		}

		//if the shadowmaps array is not empty
		if (!mShadowsMaps.empty())
		{
			//send the shadow maps to the shaders (slot 7)
			mEngine->GetContext()->PSSetShaderResources(7, static_cast<unsigned>(mShadowsMaps.size()), &mShadowsMaps[0]);
		}

		////--------------- Render Ambient Maps  ---------------////

		for (auto & o : mEngine->GetObjManager()->mObjects)
		{
			if(o->Enabled())
			{
				auto a = dynamic_cast<CDX11GameObject*>(o);
				if(a)
				{
					a->RenderToAmbientMap();
				}
			}
		}

		////--------------- Main scene rendering ---------------////

		// Set the target for rendering and select the main depth buffer.
		// If using post-processing then render to the scene texture, otherwise to the usual back buffer
		// Also clear the render target to a fixed colour and the depth buffer to the far distance

		mEngine->GetContext()->OMSetRenderTargets(1, mSceneRTV.GetAddressOf(), mDepthStencilRTV.Get());
		mEngine->GetContext()->ClearRenderTargetView(mSceneRTV.Get(), &mBackgroundColor.r);

		mEngine->GetContext()->ClearDepthStencilView(mDepthStencilRTV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		// Setup the viewport to the size of the main window
		D3D11_VIEWPORT vp;
		vp.Width = static_cast<FLOAT>(mViewportX);
		vp.Height = static_cast<FLOAT>(mViewportY);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		mEngine->GetContext()->RSSetViewports(1, &vp);

		// Render the scene from the main camera
		RenderSceneFromCamera(mCamera.get());

		// Render the scene to a depth map, for post processing
		RenderToDepthMap();

		// PostProcessing pass
		PostProcessingPass();

		mEngine->FinalizeFrameImpl();
	}

	//--------------------------------------------------------------------------------------
	// Scene Update
	//--------------------------------------------------------------------------------------

	// Update models and camera. frameTime is the time passed since the last frame
	void CDX11Scene::UpdateSceneImpl(float& frameTime)
	{

		// Post processing settings - all data for post-processes is updated every frame whether in use or not (minimal cost)

		// Set and increase the burn level (cycling back to 0 when it reaches 1.0f)
		const float burnSpeed = 0.2f;
		gPostProcessingConstants.burnHeight = fmod(gPostProcessingConstants.burnHeight + burnSpeed * frameTime, 1.0f);

		// Set and increase the amount of spiral - use a tweaked cos wave to animate
		static float wiggle = 0.0f;
		const float  wiggleSpeed = 1.0f;
		gPostProcessingConstants.spiralLevel = ((1.0f - cos(wiggle)) * 4.0f);
		wiggle += wiggleSpeed * frameTime;

		// Update heat haze timer
		gPostProcessingConstants.heatHazeTimer += frameTime;

	}

	void CDX11Scene::ResizeImpl(UINT newX, UINT newY)
	{
		//set aspect ratio with the new window size
		//broken
		mCamera->SetAspectRatio(float(newX) / float(newY));

		// set the scene viewport size to the new size
		mViewportX = newX;
		mViewportY = newY;

		// Recreate all the texture with the updated size
		InitTextures();
	}

	void CDX11Scene::InitTextures()
	{
		mTextrue = nullptr;
		mFinalTextrue = nullptr;
		mLuminanceMap = nullptr;
		mSsaoMap = nullptr;

		mSceneRTV = nullptr;
		mFinalRTV = nullptr;
		mLuminanceRTV = nullptr;
		mSsaoMapRTV = nullptr;

		mSceneSRV = nullptr;
		mFinalTextureSRV = nullptr;
		mLuminanceMapSRV = nullptr;
		mSsaoMapSRV = nullptr;

		mDepthStencil = nullptr;
		mFinalDepthStencil = nullptr;

		mDepthStencilRTV = nullptr;
		mFinalDepthStencilRTV = nullptr;

		mDepthStencilSRV = nullptr;
		mFinalDepthStencilSRV = nullptr;

		// We create a new texture for the scene with new size
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = mViewportX; // Size of the "screen"
		textureDesc.Height = mViewportY;
		textureDesc.MipLevels = 1;
		// 1 level, means just the main texture, no additional mip-maps. Usually don't use mip-maps when rendering to textures (or we would have to render every level)
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		// Indicate we will use texture as a depth buffer and also pass it to shaders
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;
		if (FAILED(mEngine->GetDevice()->CreateTexture2D(&textureDesc, NULL, mTextrue.GetAddressOf())))
		{
			throw std::runtime_error("Error creating scene texture");
		}

		if (FAILED(mEngine->GetDevice()->CreateTexture2D(&textureDesc, NULL, mFinalTextrue.GetAddressOf())))
		{
			throw std::runtime_error("Error creating scene texture");
		}

		if (FAILED(mEngine->GetDevice()->CreateTexture2D(&textureDesc, NULL, mLuminanceMap.GetAddressOf())))
		{
			throw std::runtime_error("Error creating luminance texture");
		}

		if (FAILED(mEngine->GetDevice()->CreateTexture2D(&textureDesc, NULL, mSsaoMap.GetAddressOf())))
		{
			throw std::runtime_error("Error creating ssao texture");
		}


		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;

		// We created the scene texture above, now we get a "view" of it as a render target, i.e. get a special pointer to the texture that
		// we use when rendering to it (see RenderScene function below)
		if (FAILED(mEngine->GetDevice()->CreateRenderTargetView(mTextrue.Get(), &rtvDesc, mSceneRTV.GetAddressOf())))
		{
			throw std::runtime_error("Error creating scene render target view");
		}

		if (FAILED(mEngine->GetDevice()->CreateRenderTargetView(mFinalTextrue.Get(), &rtvDesc, mFinalRTV.GetAddressOf())))
		{
			throw std::runtime_error("Error creating scene render target view");
		}

		if (FAILED(mEngine->GetDevice()->CreateRenderTargetView(mLuminanceMap.Get(), &rtvDesc, mLuminanceRTV.GetAddressOf()
		)))
		{
			throw std::runtime_error("Error creating luminance render target view");
		}

		if (FAILED(mEngine->GetDevice()->CreateRenderTargetView(mSsaoMap.Get(), &rtvDesc, mSsaoMapRTV.GetAddressOf())))
		{
			throw std::runtime_error("Error creating ssao render target view");
		}


		// We also need to send this texture (resource) to the shaders. To do that we must create a shader-resource "view"
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		if (FAILED(mEngine->GetDevice()->CreateShaderResourceView(mTextrue.Get(), &srvDesc, mSceneSRV.GetAddressOf())))
		{
			throw std::runtime_error("Error creating scene texture shader resource view");
		}

		if (FAILED(mEngine->GetDevice()->CreateShaderResourceView(mFinalTextrue.Get(), &srvDesc, mFinalTextureSRV.
			GetAddressOf())))
		{
			throw std::runtime_error("Error creating scene texture shader resource view");
		}

		if (FAILED(mEngine->GetDevice()->CreateShaderResourceView(mLuminanceMap.Get(), &srvDesc, mLuminanceMapSRV.
			GetAddressOf())))
		{
			throw std::runtime_error("Error creating luminance shader resource view");
		}

		if (FAILED(mEngine->GetDevice()->CreateShaderResourceView(mSsaoMap.Get(), &srvDesc, mSsaoMapSRV.GetAddressOf())))
		{
			throw std::runtime_error("Error creating ssao shader resource view");
		}


		//create depth stencil
		D3D11_TEXTURE2D_DESC dsDesc = {};
		dsDesc.Width = textureDesc.Width;
		dsDesc.Height = textureDesc.Height;
		dsDesc.MipLevels = 1;
		dsDesc.ArraySize = 1;
		dsDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		dsDesc.SampleDesc.Count = 1;
		dsDesc.SampleDesc.Quality = 0;
		dsDesc.Usage = D3D11_USAGE_DEFAULT;
		dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		dsDesc.CPUAccessFlags = 0;
		dsDesc.MiscFlags = 0;

		if (FAILED(mEngine->GetDevice()->CreateTexture2D(&dsDesc, NULL, mDepthStencil.GetAddressOf())))
		{
			throw std::runtime_error("Error creating depth stencil");
		}

		if (FAILED(mEngine->GetDevice()->CreateTexture2D(&dsDesc, NULL, mFinalDepthStencil.GetAddressOf())))
		{
			throw std::runtime_error("Error creating depth stencil");
		}
		//create depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.Flags = 0;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;

		if (FAILED(mEngine->GetDevice()->CreateDepthStencilView(mDepthStencil.Get(), &dsvDesc, mDepthStencilRTV.GetAddressOf
		())))
		{
			throw std::runtime_error("Error creating depth stencil view ");
		}

		if (FAILED(mEngine->GetDevice()->CreateDepthStencilView(mFinalDepthStencil.Get(), &dsvDesc, mFinalDepthStencilRTV.
			GetAddressOf())))
		{
			throw std::runtime_error("Error creating depth stencil view ");
		}

		//create the depth stencil shader view
		D3D11_SHADER_RESOURCE_VIEW_DESC dsSrvDesc = {};
		dsSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		dsSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		dsSrvDesc.Texture2D.MostDetailedMip = 0;
		dsSrvDesc.Texture2D.MipLevels = -1;

		if (FAILED(mEngine->GetDevice()->CreateShaderResourceView(mDepthStencil.Get(), &dsSrvDesc, mDepthStencilSRV.
			GetAddressOf())))
		{
			throw std::runtime_error("Error creating depth stencil shader resource view");
		}

		if (FAILED(mEngine->GetDevice()->CreateShaderResourceView(mFinalDepthStencil.Get(), &dsSrvDesc,
			mFinalDepthStencilSRV.GetAddressOf())))
		{
			throw std::runtime_error("Error creating depth stencil shader resource view");
		}
	}


	void UpdateLightsBuffer(std::deque<CLight*> o)
	{
		const auto FCB = &gPerFrameLightsConstants;

		for (auto i = 0; i < o.size(); ++i)
		{
			auto l = dynamic_cast<CGameObject*>(o[i]);
			if (*l->Enabled())
			{
				FCB->lights[i].colour = o[i]->GetColour();
				FCB->lights[i].enabled = 1;
				FCB->lights[i].position = l->Position();
				FCB->lights[i].intensity = o[i]->GetStrength();
			}
			else { FCB->lights[i].enabled = 0; }
		}
	}

	void UpdateSpotLightsBuffer(std::deque<CSpotLight*> o)
	{
		const auto FLB = &gPerFrameSpotLightsConstants;

		for (auto i = 0; i < o.size(); ++i)
		{
			auto l =o[i];
			auto ob = dynamic_cast<CGameObject*>(o[i]);
			if (*ob->Enabled())
			{
				FLB->spotLights[i].enabled = 1;
				FLB->spotLights[i].colour = l->GetColour();
				FLB->spotLights[i].pos = ob->Position();
				FLB->spotLights[i].intensity = l->GetStrength();
				FLB->spotLights[i].facing = Normalise(ob->WorldMatrix().GetRow(2));
				FLB->spotLights[i].cosHalfAngle = cos(ToRadians(l->GetConeAngle() / 2));
				FLB->spotLights[i].viewMatrix = InverseAffine(ob->WorldMatrix());
				FLB->spotLights[i].projMatrix = MakeProjectionMatrix(1.0f, ToRadians(l->GetConeAngle()));
			}
			else { FLB->spotLights[i].enabled = 0; }
		}
	}

	void UpdateDirLightsBuffer(std::deque<CDirectionalLight*> o)
	{
		const auto FLB = &gPerFrameDirLightsConstants;

		for (auto i = 0; i < o.size(); ++i)
		{
			auto l = o[i];
			auto ob = dynamic_cast<CGameObject*>(o[i]);
			if (*ob->Enabled())
			{
				FLB->dirLights[i].enabled = 1;
				FLB->dirLights[i].colour = l->GetColour();
				FLB->dirLights[i].facing = ob->Position();
				FLB->dirLights[i].viewMatrix = InverseAffine(ob->WorldMatrix());
				FLB->dirLights[i].projMatrix = MakeOrthogonalMatrix(l->GetWidth(), l->GetWidth(), l->GetNearClip(), l->GetFarClip());
				FLB->dirLights[i].intensity = l->GetStrength();
			}
			else { FLB->dirLights[i].enabled = 0; }
		}
	}

	void UpdatePointLightsBuffer(std::deque<CPointLight*> o)
	{
		for (unsigned long long i = 0; i < o.size(); ++i)
		{
			auto l = o[i];
			auto ob = dynamic_cast<CGameObject*>(o[i]);
			if (*ob->Enabled())
			{
				gPerFramePointLightsConstants.pointLights[i].enabled = 1;
				gPerFramePointLightsConstants.pointLights[i].colour = l->GetColour();
				gPerFramePointLightsConstants.pointLights[i].intensity = l->GetStrength();
				gPerFramePointLightsConstants.pointLights[i].position = ob->Position();

				for (auto j = 0; j < 6; ++j)
				{
					CVector3 rot = l->mSides[j];

					ob->SetRotation(rot * PI);

					gPerFramePointLightsConstants.pointLights[i].viewMatrices[j] = InverseAffine(ob->WorldMatrix());
				}
				//since they are all the same we just need one projection matrix
				gPerFramePointLightsConstants.pointLights[i].projMatrix = MakeProjectionMatrix(1.0f, ToRadians(90.0f));
			}
			else { gPerFramePointLightsConstants.pointLights[i].enabled = 0; }
		}
	}

	void UpdateAllBuffers(CGameObjectManager* g)
	{
		UpdateLightsBuffer(g->mLights);
		UpdateSpotLightsBuffer(g->mSpotLights);
		UpdateDirLightsBuffer(g->mDirLights);
		UpdatePointLightsBuffer(g->mPointLights);

		// Update number of lights
		gPerFrameConstants.nLights = (float)g->mLights.size();
		gPerFrameConstants.nDirLight = (float)g->mDirLights.size();
		gPerFrameConstants.nSpotLights = (float)g->mSpotLights.size();
		gPerFrameConstants.nPointLights = (float)g->mPointLights.size();
	}
}
