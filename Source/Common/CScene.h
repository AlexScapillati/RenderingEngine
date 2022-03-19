#pragma once

//--------------------------------------------------------------------------------------
// Scene Generic Class
// Static Polymorphism
//--------------------------------------------------------------------------------------

#include <memory>
#include <sstream>
#include <stdexcept>

#include "Camera.h"
#include "CLight.h"
#include "imgui.h"
#include "LevelImporter.h"
#include "../Engine.h"
#include "../Window.h"
#include "../Math/CVector2.h"
#include "../Math/CVector3.h"
#include "..\Utility/ColourRGBA.h"

class CWindow;
class CCamera;

template <typename Impl>
class CScene
{
<<<<<<< Updated upstream

public:

	~CScene() = default;

	//--------------------------------------------------------------------------------------
	// Scene Implementation Independent Stuff
	//--------------------------------------------------------------------------------------

	CScene(IEngine<Impl>* engine, std::string fileName)
	{

=======

public:

	~CScene() = default;

	//--------------------------------------------------------------------------------------
	// Scene Implementation Independent Stuff
	//--------------------------------------------------------------------------------------

	CScene(IEngine<Impl>* engine, std::string fileName)
	{

>>>>>>> Stashed changes
		mEngine = engine;
		mWindow = mEngine->GetWindow();
		mFileName = fileName;

		//--------------------------------------------------------------------------------------
		// Scene Geometry and Layout
		//--------------------------------------------------------------------------------------

<<<<<<< Updated upstream
<<<<<<< Updated upstream
		virtual void Save(std::string fileName = "");
		virtual void Resize(UINT newX, UINT newY) = 0;
		virtual void PostProcessingPass() = 0;
		virtual void RenderToDepthMap() = 0;
		virtual void DisplayPostProcessingEffects() = 0; // TODO: Remove
		

		//--------------------------------------------------------------------------------------
		// Getters 
		//--------------------------------------------------------------------------------------
		
		auto  GetViewportSize() const { return CVector2((float)mViewportX, (float)mViewportY); }
		auto  GetViewportX() const { return mViewportX; }
		auto  GetViewportY() const { return mViewportY; }
		CCamera*  GetCamera() const { return mCamera.get(); }
		void SetCamera(CCamera* c) { mCamera.reset(c); }
		auto& GetLockFps() { return mLockFPS; }
		auto& GetBackgroundCol() { return mBackgroundColor; }

	private:
		IEngine* mEngine;

	protected:
		//--------------------------------------------------------------------------------------
		// Private Variables
		//--------------------------------------------------------------------------------------
		
		std::unique_ptr<CCamera>            mCamera;

		// Lock FPS to monitor refresh rate, which will typically set it to 60fps. Press 'p' to toggle to full fps
		bool         mLockFPS     = true;
		UINT         mViewportX   = 1920;
		UINT         mViewportY   = 1080;
		int          mPcfSamples  = 4;
		CGameObject* mSelectedObj = nullptr;
		CWindow*     mWindow      = nullptr;
		std::string  mFileName;

		// Additional light information
		CVector3 gAmbientColour   = { 0.03f,0.03f,0.04f }; // Background level of light (slightly bluish to match the far background, which is dark blue)
		ColourRGBA mBackgroundColor = { 0.3f,0.3f,0.4f,1.0f };
		float    gSpecularPower   = 256; // Specular power //will be removed since it will be dependent on the material
=======
=======
>>>>>>> Stashed changes
		try
		{

			if (fileName.empty())
			{
				//--------------------------------------------------------------------------------------
				// Scene Geometry and Layout
				//--------------------------------------------------------------------------------------

				mBackgroundColor = { 0.1f,0.1f,0.1f,1.0f };

				mCamera = std::make_unique<CCamera>();
				mCamera->SetPosition({ 0, 10.0f, -4.0f });

				auto ground = mEngine->CreateObject("Hills.x", "Ground", "GrassDiffuseSpecular.dds");
				auto cube = mEngine->CreateObject("Cube.x", "Cube", "Mossy.png");
				auto sky = mEngine->CreateSky("Stars.x", "sky", "Stars.jpg");
				cube->SetPosition({ 0.0f,10.0f,20.0f });
				sky->SetScale(1000);

				auto light = mEngine->CreatePointLight("Light.x", "PointLight", "Flare.jpg", { 1,1,1 }, 1000);
				light->SetPosition({ 10.f,20.f,30.f });
				light->SetRotation({ 90.f,0.f,0.f });

				auto l = mEngine->CreateLight("Light.x", "Light", "Flare.jpg", { 1,1,1 }, 1000);
				l->SetPosition({ 10.f,20000.f,30.f });
			}
			else
			{
				CLevelImporter importer(engine);

				importer.LoadScene(fileName);
			}
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error(e.what());
		}
	}

	// frameTime is the time passed since the last frame
	void UpdateScene(float& frameTime)
	{
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
			const auto windowTitle = "DirectX - Game Engine Test " + frameTimeMs.str() +
				"ms, FPS: " + std::to_string(static_cast<int>(1 / avgFrameTime + 0.5f));
			SetWindowTextA(mEngine->GetWindow()->GetHandle(), windowTitle.c_str());
			totalFrameTime = 0;
			frameCount = 0;
		}
	}

	void Save(std::string fileName = "")
	{
		CLevelImporter i(mEngine);
		i.SaveScene(fileName);
	}


   //--------------------------------------------------------------------------------------
   // Getters 
   //--------------------------------------------------------------------------------------

	auto  GetViewportSize() const { return CVector2((float)mViewportX, (float)mViewportY); }
	auto  GetViewportX() const { return mViewportX; }
	auto  GetViewportY() const { return mViewportY; }
	CCamera* GetCamera() const { return mCamera.get(); }
	void SetCamera(CCamera* c) { mCamera.reset(c); }
	auto& GetLockFps() { return mLockFPS; }
	auto& GetBackgroundCol() { return mBackgroundColor; }


	//--------------------------------------------------------------------------------------
	// Scene Implementation Dependent Functions
	//--------------------------------------------------------------------------------------

	// Returns the generated scene texture
	void RenderScene(float& frameTime) { impl()->RenderSceneImpl(frameTime); }

	void RenderSceneFromCamera(CCamera* camera) { impl()->RenderSceneFromCameraImpl(camera); }

	ImTextureID GetTextureSRV() { return impl()->GetTextureSRVImpl(); };

	//--------------------------------------------------------------------------------------
	// Public Functions
	//--------------------------------------------------------------------------------------


	void Resize(UINT newX, UINT newY) { impl()->ResizeImpl(newX, newY); }
	void PostProcessingPass() { impl()->PostProcessingPassImpl(); }
	void RenderToDepthMap() { impl()->RenderToDepthMapImpl(); }
	void DisplayPostProcessingEffects() { impl()->DisplayPostProcessingEffectsImpl(); } // TODO: Remove



private:

	IEngine<Impl>* mEngine;

	Impl* impl() { return static_cast<Impl*>(this); }

protected:
	//--------------------------------------------------------------------------------------
	// Private Variables
	//--------------------------------------------------------------------------------------

	std::unique_ptr<CCamera>            mCamera;

	// Lock FPS to monitor refresh rate, which will typically set it to 60fps. Press 'p' to toggle to full fps
	bool         mLockFPS = true;
	UINT         mViewportX = 1920;
	UINT         mViewportY = 1080;
	int          mPcfSamples = 4;
	CGameObject* mSelectedObj = nullptr;
	CWindow* mWindow = nullptr;
	std::string  mFileName;

	// Additional light information
	CVector3 gAmbientColour = { 0.03f,0.03f,0.04f }; // Background level of light (slightly bluish to match the far background, which is dark blue)
	ColourRGBA mBackgroundColor = { 0.3f,0.3f,0.4f,1.0f };
	float    gSpecularPower = 256; // Specular power //will be removed since it will be dependent on the material
<<<<<<< Updated upstream
>>>>>>> Stashed changes
=======
>>>>>>> Stashed changes
};
