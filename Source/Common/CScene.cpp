#include "CScene.h"

#include <ios>
#include <sstream>
#include <stdexcept>
#include <string>

#include "Camera.h"
#include "CGameObject.h"
#include "LevelImporter.h"
#include "../Engine.h"
#include "../Utility/Input.h"
#include "../Window.h"
#include "../Common/CLight.h"

CScene::CScene(IEngine* engine, std::string fileName)
{

	mEngine = engine;
	mWindow = mEngine->GetWindow();
	mFileName = fileName;

	//--------------------------------------------------------------------------------------
	// Scene Geometry and Layout
	//--------------------------------------------------------------------------------------

	try
	{
		mObjManager = std::make_unique<CGameObjectManager>(engine);

		if (fileName.empty())
		{
			//--------------------------------------------------------------------------------------
			// Scene Geometry and Layout
			//--------------------------------------------------------------------------------------

			mObjManager = std::make_unique<CGameObjectManager>(engine);

			mCamera = std::make_unique<CCamera>();
			mCamera->SetPosition({ 0, 10.0f, -4.0f });

			auto ground = mEngine->CreateObject("Hills.x", "Ground", "GrassDiffuseSpecular.dds");
			auto cube = mEngine->CreateObject("Cube.x", "Cube", "Mossy.png");
			auto sky = mEngine->CreateSky("Stars.x", "sky", "Stars.jpg");
			cube->SetPosition({ 0.0f,10.0f,20.0f });

			auto light = mEngine->CreateLight("Light.x", "Light", "Flare.jpg", { 1,1,1 }, 1000);
			light->SetPosition({ 10.f,20.f,30.f });

			mObjManager->AddObject(ground);
			mObjManager->AddObject(cube);
			mObjManager->AddLight(light);
			mObjManager->AddSky(sky);

		}
		else
		{
			CLevelImporter importer(engine);

			importer.LoadScene(fileName, this);
		}
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(e.what());
	}
}


void CScene::UpdateScene(float& frameTime)
{
	// Toggle FPS limiting
	if (KeyHit(Key_P))
	{
		mLockFPS = !mLockFPS;
	}
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
	
>>>>>>> parent of 100d753 (Merge pull request #3 from AlexScapillati/TryingPolymorphism)
=======
	
>>>>>>> parent of b0bd427 (Up)
=======
>>>>>>> parent of 7bb1619 (Merge branch 'main' into TryingPolymorphism)

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
			"ms, FPS: " + std::to_string(static_cast<int>(1 / avgFrameTime + 0.5f));
		SetWindowTextA(mEngine->GetWindow()->GetHandle(), windowTitle.c_str());
		totalFrameTime = 0;
		frameCount = 0;
	}
}

void CScene::Save(std::string fileName)
{
	CLevelImporter i(mEngine);
	i.SaveScene(fileName, this);
}