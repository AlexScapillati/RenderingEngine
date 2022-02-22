#pragma once

//--------------------------------------------------------------------------------------
// Scene geometry and layout preparation
// Scene rendering & update
//--------------------------------------------------------------------------------------

#pragma once

#include <intsafe.h>

#include "CGameObjectManager.h"

#include "..\Utility/ColourRGBA.h"
#include <memory>

#include "Camera.h"
#include "CPostProcess.h"
#include "imgui.h"
#include "../Math/CVector2.h"
#include "../Math/CVector3.h"
class CWindow;
class CCamera;

class CScene
{
	
	public:
		virtual ~CScene() = default;


		CScene(IEngine* engine, std::string fileName);
		CScene(IEngine* engine);

		virtual void RenderSceneFromCamera(CCamera* camera) = 0;

		//--------------------------------------------------------------------------------------
		// Scene Render and Update
		//--------------------------------------------------------------------------------------

		// Returns the generated scene texture
		virtual void RenderScene(float& frameTime) = 0;

		// frameTime is the time passed since the last frame
		virtual void UpdateScene(float& frameTime);

		virtual ImTextureID GetTextureSRV() = 0;

		//--------------------------------------------------------------------------------------
		// Public Functions
		//--------------------------------------------------------------------------------------

		virtual void Save(std::string fileName = "");
		virtual void Resize(UINT newX, UINT newY);
		virtual void PostProcessingPass();
		virtual void RenderToDepthMap();
		virtual void DisplayPostProcessingEffects(); // TODO: Remove
		

		//--------------------------------------------------------------------------------------
		// Getters 
		//--------------------------------------------------------------------------------------

		auto  GetObjectManager() const { return mObjManager.get(); }
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

		std::unique_ptr<CGameObjectManager> mObjManager;
		std::unique_ptr<CCamera>            mCamera;

		// Lock FPS to monitor refresh rate, which will typically set it to 60fps. Press 'p' to toggle to full fps
		bool         mLockFPS     = true;
		UINT         mViewportX   = 1024;
		UINT         mViewportY   = 720;
		int          mPcfSamples  = 4;
		CGameObject* mSelectedObj = nullptr;
		CWindow*     mWindow      = nullptr;
		std::string  mFileName;

		// Additional light information
		CVector3 gAmbientColour   = { 0.03f,0.03f,0.04f }; // Background level of light (slightly bluish to match the far background, which is dark blue)
		ColourRGBA mBackgroundColor = { 0.3f,0.3f,0.4f,1.0f };
		float    gSpecularPower   = 256; // Specular power //will be removed since it will be dependent on the material
};
