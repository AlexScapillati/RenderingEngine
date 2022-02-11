#include "DX12Scene.h"

#include <sstream>

#include "DX12Importer.h"

#include "..\Window.h"

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

	mPcfSamples = 4;

	mObjManager = std::make_unique<CDX12GameObjectManager>(mEngine);

	gAmbientColour = { 0.03f, 0.03f, 0.04f };
	gSpecularPower = 256; // Specular power //will be removed since it will be dependent on the material
	mLockFPS = true;
	mBackgroundColor = { 0.3f, 0.3f, 0.4f, 1.0f };

	try
	{
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

	mViewportX = 1024;
	mViewportY = 720;

	mPcfSamples = 4;

	//mObjManager = std::make_unique<CDX12GameObjectManager>(mEngine);

	gAmbientColour = { 0.03f, 0.03f, 0.04f };
	gSpecularPower = 256; // Specular power //will be removed since it will be dependent on the material
	mLockFPS = true;
	mBackgroundColor = { 0.3f, 0.3f, 0.4f, 1.0f };

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
		D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM,
			GetViewportX(),
			GetViewportY(),
			1, 1, 1, 0,
			D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

		mSceneTexture = std::make_unique <CDX12RenderTarget>(mEngine, desc);
	}

	//Creating the depth texture
	{
		const CD3DX12_RESOURCE_DESC        depth_texture(
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			0,
			GetViewportX(),
			GetViewportY(),
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
				IID_PPV_ARGS(&mDepthStencils[i])
			);

			NAME_D3D12_OBJECT_INDEXED(mDepthStencils, i);

			assert(result == S_OK && "CREATING THE DEPTH STENCIL FAILED");

			CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(mEngine->mDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
			dsvHandle.Offset(i, mEngine->mDSVSize);

			mEngine->mDevice->CreateDepthStencilView(mDepthStencils[i].Get(), nullptr, dsvHandle);
		}
	}
}


void CDX12Scene::InitScene()
{
	mCamera = std::make_unique<CCamera>();
	mCamera->SetPosition({ 0, 10.0f, -4.0f });

	std::string map    = "GrassDiffuseSpecular.dds";
	const auto  ground = new CDX12GameObject(mEngine, "Hills.x", "Ground", map);

	map             = "Mossy.png";
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
	ID3D12DescriptorHeap* ppHeaps[] = { mEngine->mCBVDescriptorHeap.Get() };
	mEngine->mCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// Render the models from the camera pov
	RenderSceneFromCamera(mCamera.get());

	// Render the scene to depth map
	RenderToDepthMap();

	// Post processing pass
}

void CDX12Scene::RenderSceneFromCamera(CCamera* camera) const
{
	// Set camera matrices in the constant buffer and send over to GPU

	auto& perFrameConstants = mEngine->mPerFrameConstants;
	perFrameConstants.cameraMatrix = camera->WorldMatrix();
	perFrameConstants.viewMatrix = camera->ViewMatrix();
	perFrameConstants.projectionMatrix = camera->ProjectionMatrix();
	perFrameConstants.viewProjectionMatrix = camera->ViewProjectionMatrix();
	perFrameConstants.ambient = gAmbientColour;
	perFrameConstants.specularPower = gSpecularPower;

	// todo: mGameobjectManager->updateLightsbuffers

	const auto light = mObjManager->mLights.front();

	const SLight lightInfo = { light->Position(),static_cast<float>(*light->Enabled()),light->Colour(), light->Strength() };
	perFrameConstants.lights[0] = lightInfo;

	mEngine->CopyBuffers();

	mObjManager->RenderAllObjects();

	ImGui::Checkbox("Use custom values", &perFrameConstants.customValues);

	ImGui::SliderFloat("roughness", &perFrameConstants.roughness, 0.f, 1.f);
	ImGui::SliderFloat("metalness", &perFrameConstants.metalness, 0.f, 1.f);


	const ImVec2 size = { 128,128 };

	for (auto i = 1; i < mEngine->mNumTextures; ++i)
	{
		const CD3DX12_GPU_DESCRIPTOR_HANDLE hGPUDescriptor(mEngine->mSRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), i, mEngine->mSRVSize);

		ImGui::Image((ImTextureID)hGPUDescriptor.ptr, size);
	}
	

}

void CDX12Scene::UpdateScene(float& frameTime)
{
	// Toggle FPS limiting
	if (KeyHit(Key_P))
	{
		mLockFPS = !mLockFPS;
	}

	GetCamera()->Control(frameTime, Key_W, Key_S, Key_A, Key_D, Key_W, Key_S, Key_A, Key_D);

	const auto mLight = mObjManager->mLights.front();

	auto s = mLight->Strength();
	if (ImGui::DragFloat("Light Strength", &s))
	{
		mLight->SetStrength(s);
	}

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

ImTextureID CDX12Scene::GetTextureSRV()
{
	return ImTextureID(mSceneTexture->mCpuSRVDescriptorHandle.ptr);
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

	mSceneTexture.release();

	for (int i = 0; i < CDX12Engine::mNumFrames; ++i)
	{
		mDepthStencils[i].Reset();
	}

	InitFrameDependentStuff();
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
