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
#include "../DX12/DX12Engine.h"
#include "../DX12/Objects/DX12Light.h"
#include "../DX11/Objects/DX11Light.h"

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

		CLevelImporter importer(engine);

		importer.LoadScene(std::move(fileName), this);
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(e.what());
	}
}

CScene::CScene(IEngine* engine)
{
	mEngine = engine;
	mWindow = mEngine->GetWindow();

	//--------------------------------------------------------------------------------------
	// Scene Geometry and Layout
	//--------------------------------------------------------------------------------------
	mCamera = std::make_unique<CCamera>();
	mCamera->SetPosition({ 0, 10.0f, -4.0f });

	auto ground = mEngine->CreateObject("Hills.x", "Ground", "GrassDiffuseSpecular.dds");
	auto cube = mEngine->CreateObject("Cube.x", "Cube", "Mossy.png");
	auto light = mEngine->CreateLight("Light.x", "Light", "Flare.jpg", { 1,1,1 }, 1000);
	
	light->SetPosition({ 10.f,20.f,30.f });
	cube->SetPosition({ 0.0f,10.0f,20.0f });
	mObjManager->AddObject(ground);
	mObjManager->AddObject(cube);
	mObjManager->AddLight(light);
}

void CScene::UpdateScene(float& frameTime)
{
	// Toggle FPS limiting
	if (KeyHit(Key_P))
	{
		mLockFPS = !mLockFPS;
	}

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
		const auto windowTitle = "DirectX 11 - Game Engine Test " + frameTimeMs.str() +
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

void CScene::Resize(UINT newX, UINT newY)
{
}

void CScene::PostProcessingPass()
{
}

void CScene::RenderToDepthMap()
{
}

void CScene::DisplayPostProcessingEffects()
{
}
