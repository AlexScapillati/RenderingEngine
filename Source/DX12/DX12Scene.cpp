#include "DX12Scene.h"

#include "DX12DescriptorHeap.h"
#include "DX12GameObjectManager.h"
#include "DX12Importer.h"
#include "DX12Texture.h"

#include "Objects/DX12Light.h"

#include "../Window.h"
#include "../Utility/Input.h"

CDX12Scene::CDX12Scene(CDX12Engine* engine)
{
	mWindow = engine->GetWindow();
	mEngine = engine;

	mFileName = "";

	//--------------------------------------------------------------------------------------
	// Scene Geometry and Layout
	//--------------------------------------------------------------------------------------

	mCamera = nullptr;
	mSelectedObj = nullptr;

	mViewportX = 1024;
	mViewportY = 720;

	mLockFPS = true;

	try
	{

		mObjManager = std::make_unique<CDX12GameObjectManager>(mEngine);

		InitFrameDependentStuff();

		// Initialize scene
		InitScene();

	}
	catch (const std::runtime_error& e)
	{
		throw std::runtime_error(e.what());
	}

}

CDX12Scene::CDX12Scene(CDX12Engine* engine, std::string fileName)
{
	mWindow = engine->GetWindow();
	mEngine = engine;

	mFileName = fileName;

	//--------------------------------------------------------------------------------------
	// Scene Geometry and Layout
	//--------------------------------------------------------------------------------------

	mCamera = nullptr;
	mSelectedObj = nullptr;

	mViewportX = 1240;
	mViewportY = 720;

	gAmbientColour = { 0.03f, 0.03f, 0.04f };
	mLockFPS = true;

	//--------------------------------------------------------------------------------------
	// Initialise scene geometry
	//--------------------------------------------------------------------------------------

	////--------------- Load meshes ---------------////

	try
	{

		InitFrameDependentStuff();

		//LoadPostProcessingImages();

		// Create object manager
		mObjManager = std::make_unique<CDX12GameObjectManager>(mEngine);

		// Load the scene
		CDX12Importer importer(mEngine);

		importer.LoadScene(std::move(fileName), this);
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(e.what());
	}

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
		clear_value.Format = DXGI_FORMAT_D32_FLOAT;
		clear_value.DepthStencil.Depth = 1.0f;
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

			auto dsv = mEngine->mDSVDescriptorHeap->Add().mCpu;

			mEngine->mDevice->CreateDepthStencilView(mEngine->mDepthStencils[i].Get(), nullptr, dsv);
		}
	}
}


void CDX12Scene::InitScene()
{
	mCamera = std::make_unique<CCamera>();
	mCamera->SetPosition({ 0, 10.0f, -4.0f });

	std::string map = "GrassDiffuseSpecular.dds";
	const auto  ground = new CDX12GameObject(mEngine, "Hills.x", "Ground", map);

	map = "Mossy.png";
	const auto cube = new CDX12GameObject(mEngine, "Cube.x", "Cube", map);
	cube->SetPosition({ 0.0f,10.0f,20.0f });

	const auto light = new CDX12Light(mEngine, "Light.x", "Light", "Flare.jpg");
	light->SetStrength(10);
	light->SetPosition({ 10.f,20.f,30.f });

	mObjManager->AddObject(ground);
	mObjManager->AddObject(cube);
	mObjManager->AddLight(light);
}

void CDX12Scene::RenderScene(float& frameTime)
{
	const FLOAT clearColor[] = { 0.4f,0.6f,0.9f,1.0f };

	const auto rtv = mSceneTexture->mRTVHandle.mCpu;

	auto dsv = mEngine->mDSVDescriptorHeap->Get(mEngine->mCurrentBackBufferIndex).mCpu;

	mSceneTexture->Barrier(D3D12_RESOURCE_STATE_RENDER_TARGET);

	auto vp = CD3DX12_VIEWPORT(
		0.0f,
		0.0f,
		static_cast<FLOAT>(mViewportX),
		static_cast<FLOAT>(mViewportY));

	auto sr = CD3DX12_RECT(
		0,
		0,
		static_cast<LONG>(mViewportX),
		static_cast<LONG>(mViewportY));

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

void CDX12Scene::RenderSceneFromCamera(CCamera* camera) const
{
	// Set camera matrices in the constant buffer and send over to GPU

	mEngine->mCBVDescriptorHeap->Set();

	auto& perFrameConstants = mEngine->mPerFrameConstants;
	perFrameConstants.cameraMatrix = camera->WorldMatrix();
	perFrameConstants.viewMatrix = camera->ViewMatrix();
	perFrameConstants.projectionMatrix = camera->ProjectionMatrix();
	perFrameConstants.viewProjectionMatrix = camera->ViewProjectionMatrix();
	perFrameConstants.ambient = gAmbientColour;

	// todo: mGameobjectManager->updateLightsbuffers

	if (!mObjManager->mLights.empty())
	{
		const auto light = mObjManager->mLights.front();
		const SLight lightInfo = { light->Position(),static_cast<float>(*light->Enabled()),light->Colour(), light->Strength() };
		mEngine->mPerFrameLights.lights[0] = lightInfo;
	}

	mEngine->CopyBuffers();

	PIXBeginEvent(mEngine->mCommandList.Get(), 0, "Rendering");

	mObjManager->RenderAllObjects();

	PIXEndEvent(mEngine->mCommandList.Get());
}

void CDX12Scene::UpdateScene(float& frameTime)
{
	// Toggle FPS limiting
	if (KeyHit(Key_P))
	{
		mLockFPS = !mLockFPS;
	}

	GetCamera()->Control(frameTime, Key_W, Key_S, Key_A, Key_D, Key_W, Key_S, Key_A, Key_D);

	// Show frame time / FPS in the window title //
	const auto   fpsUpdateTime = 0.05f; // How long between updates (in seconds)
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
		const auto windowTitle = "DirectX 12 - Game Engine Test " + frameTimeMs.str() +
			"ms, FPS: " + std::to_string(static_cast<int>(1 / avgFrameTime + 0.5f));
		SetWindowTextA(mWindow->GetHandle(), windowTitle.c_str());
		totalFrameTime = 0;
		frameCount = 0;
	}
}

auto CDX12Scene::GetTextureSrv() const -> ImTextureID
{
	return reinterpret_cast<ImTextureID>(mSceneTexture->mHandle.mGpu.ptr);
}

void CDX12Scene::Save(std::string fileName)
{
	if (fileName.empty()) fileName = mFileName;

	CDX12Importer importer(mEngine);

	importer.SaveScene(fileName, this);
}

void CDX12Scene::Resize(UINT newX, UINT newY) {

	mCamera->SetAspectRatio(float(newX) / float(newY));

	mViewportX = newX;
	mViewportY = newY;

	auto desc = mSceneTexture->mResource->GetDesc();
	desc.Width = newX;
	desc.Height = newY;

	mSceneTexture->Reset(desc);

	for (int i = 0; i < CDX12Engine::mNumFrames; ++i)
	{
		mEngine->mDepthStencils[i].Reset();
	}

	//Creating the depth texture
	{
		const CD3DX12_RESOURCE_DESC depth_texture(
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			0,
			desc.Width,
			desc.Height,
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
		clear_value.Format = DXGI_FORMAT_D32_FLOAT;
		clear_value.DepthStencil.Depth = 1.0f;
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

			auto dsv = mEngine->mDSVDescriptorHeap->Add().mCpu;

			mEngine->mDevice->CreateDepthStencilView(mEngine->mDepthStencils[i].Get(), nullptr, dsv);
		}
	}
}

void CDX12Scene::PostProcessingPass()
{
}

void CDX12Scene::RenderToDepthMap()
{

}

void CDX12Scene::DisplayPostProcessingEffects()
{
}

CDX12GameObjectManager* CDX12Scene::GetObjectManager() const { return mObjManager.get(); }

CVector2 CDX12Scene::GetViewportSize() const { return CVector2((float)mViewportX, (float)mViewportY); }

UINT CDX12Scene::GetViewportX() const { return mViewportX; }

UINT CDX12Scene::GetViewportY() const { return mViewportY; }

CCamera* CDX12Scene::GetCamera() const { return mCamera.get(); }

bool& CDX12Scene::GetLockFps() { return mLockFPS; }
