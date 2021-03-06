#include "CScene.h"

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

#include "../DX11/DX11Engine.h"
#include "../DX12/DX12Engine.h"

CScene::CScene(IEngine* engine, const std::string& fileName)
{

	mEngine = engine;
	mWindow = mEngine->GetWindow();
	mFileName = fileName;
	mCamera = std::make_unique<CCamera>();

	//--------------------------------------------------------------------------------------
	// Scene Geometry and Layout
	//--------------------------------------------------------------------------------------

	try
	{

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
			//auto rocks = mEngine->CreateObject("BlackGalaxyGraniteBrickbondTiles_3D Assembly", "Rocks");
			auto sphere = mEngine->CreateObject("Steel_sfcqbiec_Surface", "Sphere");
			//auto ground = mEngine->CreateObject("Ground", "Ground");

			//auto clementine = mEngine->CreateObject("Clementine_vffmejaiw", "Clementine");
			//clementine->SetScale({1,1,1});

			//mEngine->CreateObject("Roman Statue_tfprbilda_3D Asset","statue",CVector3(10,20,0));

			////auto light = mEngine->CreateSpotLight("Light.x", "PointLight", "Flare.jpg", { 1,1,1 }, 1000);
			////light->SetPosition({ 20,10.f,10.f });
			////light->SetRotation({ ToRadians(180),0,0 });

			auto l = mEngine->CreateLight("Light.x", "Light", "Flare.jpg", { 1,1,1 }, 10000);
			l->SetPosition({ 10.f,50.f,30.f });

			//auto l1 = mEngine->CreateLight("Light.x", "Light", "Flare.jpg", { 1,1,1 }, 1000);
			//l1->SetPosition({ -10.f,-50.f,-30.f });
			//
			//auto l3 = mEngine->CreateLight("Light.x", "Light", "Flare.jpg", { 1,1,1 }, 1000);
			//l3->SetPosition({ 0.f,0.f,0.f });
		}
		else
		{
			CLevelImporter::LoadScene(fileName, mEngine);
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

	mCamera->Control(frameTime);
	

	// Show frame time / FPS in the window title //
	const auto   fpsUpdateTime = 0.5f; // How long between updates (in seconds)
	static float totalFrameTime = 0;
	static auto  frameCount = 0;
	totalFrameTime += frameTime;
	++frameCount;
	if (totalFrameTime > fpsUpdateTime)
	{
		std::string engineTitle;

		if(dynamic_cast<DX12::CDX12Engine*>(mEngine))
		{
			engineTitle = "DirectX12";
		}
		else if(dynamic_cast<DX11::CDX11Engine*>(mEngine))
		{
			engineTitle = "DirectX11";
		}

		// Displays FPS rounded to nearest int, and frame time (more useful for developers) in milliseconds to 2 decimal places
		const auto         avgFrameTime = totalFrameTime / frameCount;
		std::ostringstream frameTimeMs;
		frameTimeMs.precision(2);
		frameTimeMs << std::fixed << avgFrameTime * 1000;
		const auto windowTitle = engineTitle  + " - Game Engine Test " + frameTimeMs.str() +
			"ms, GPU: " + std::to_string(static_cast<int>(1 / avgFrameTime + 0.5f));
		SetWindowTextA(mEngine->GetWindow()->GetHandle(), windowTitle.c_str());
		totalFrameTime = 0;
		frameCount = 0;
	}
}

void CScene::Save(std::string fileName)
{
	CLevelImporter::SaveScene(fileName);
}