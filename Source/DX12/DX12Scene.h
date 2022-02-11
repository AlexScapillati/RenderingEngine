#pragma once
#include <string>

#include "DX12Engine.h"
#include "DX12GameObjectManager.h"
#include "imgui.h"

#include "../Common/Camera.h"

#include "../Utility/ColourRGBA.h"
#include "../Math/CVector2.h"
#include "../Math/CVector3.h"


	class CDX12Scene
	{
	public:

		//--------------------------------------------------------------------------------------
		// Constructors
		//--------------------------------------------------------------------------------------

		CDX12Scene(CDX12Engine* engine);


		CDX12Scene(CDX12Engine* engine,
					std::string  fileName);

		//--------------------------------------------------------------------------------------
		// Initialization
		//--------------------------------------------------------------------------------------

		void InitFrameDependentStuff();

		void InitScene();

		//--------------------------------------------------------------------------------------
		// Scene Render and Update
		//--------------------------------------------------------------------------------------

		// Returns the generated scene texture
		void RenderScene(float& frameTime);

		// frameTime is the time passed since the last frame
		void UpdateScene(float& frameTime);


		void        RenderSceneFromCamera(CCamera* camera) const;
		ImTextureID GetTextureSRV();

		//--------------------------------------------------------------------------------------
		// DirectX
		//--------------------------------------------------------------------------------------

		CD3DX12_VIEWPORT mViewport;
		CD3DX12_RECT mScissorRect;

		std::unique_ptr<CDX12RenderTarget> mSceneTexture;

		ComPtr<ID3D12Resource> mDepthStencils[CDX12Engine::mNumFrames];

		//--------------------------------------------------------------------------------------
		// Public Variables TODO REMOVE BIG NONO
		//--------------------------------------------------------------------------------------

		std::string mDefaultVs;
		std::string mDefaultPs;

		std::unique_ptr<CDX12GameObjectManager> mObjManager;

		std::unique_ptr<CCamera> mCamera;

		ColourRGBA mBackgroundColor;

		UINT mViewportX;
		UINT mViewportY;

		int mPcfSamples;


		//--------------------------------------------------------------------------------------
		// Public Functions
		//--------------------------------------------------------------------------------------

		void Save(std::string fileName = "");
		void Resize(UINT newX,
			UINT newY);
		void PostProcessingPass();
		void RenderToDepthMap();
		void DisplayPostProcessingEffects(); // TODO: Remove


		//--------------------------------------------------------------------------------------
		// Getters 
		//--------------------------------------------------------------------------------------


		auto  GetObjectManager() const { return mObjManager.get(); }
		auto  GetViewportSize() const { return CVector2((float)mViewportX, (float)mViewportY); }
		auto  GetViewportX() const { return mViewportX; }
		auto  GetViewportY() const { return mViewportY; }
		auto  GetCamera() const { return mCamera.get(); }
		auto& GetLockFps() { return mLockFPS; }


	private:
		//--------------------------------------------------------------------------------------
		// Scene Data
		//--------------------------------------------------------------------------------------


		CDX12Engine* mEngine = nullptr;
		CWindow* mWindow = nullptr;

		std::string mFileName;

		// Lock FPS to monitor refresh rate, which will typically set it to 60fps. Press 'p' to toggle to full fps
		bool mLockFPS = true;

		// Variables controlling light1's orbiting of the particle emitter
		float gCameraOrbitRadius;
		float gCameraOrbitSpeed;

		// Additional light information
		CVector3 gAmbientColour;
		// Background level of light (slightly bluish to match the far background, which is dark blue)
		float gSpecularPower; // Specular power controls shininess - same for all models in this app


		CDX12GameObject* mSelectedObj;
	};