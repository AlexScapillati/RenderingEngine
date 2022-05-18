#pragma once

#include "DX12Common.h"
#include "imgui.h"
#include "../Utility/ColourRGBA.h"
#include "../Common/Camera.h"



class CWindow;

namespace DX12
{
	class CDX12DescriptorHeap;
	class CDX12RenderTarget;
	class CDX12Engine;
	class CDX12AmbientMap;
	class CDX12DepthStencil;
	class CDX12GameObject;

	class CDX12Scene
	{
	public:

		CDX12Scene() = delete;
		CDX12Scene(const CDX12Scene&) = delete;
		CDX12Scene(const CDX12Scene&&) = delete;
		CDX12Scene& operator=(const CDX12Scene&) = delete;
		CDX12Scene& operator=(const CDX12Scene&&) = delete;

		//--------------------------------------------------------------------------------------
		// Constructors
		//--------------------------------------------------------------------------------------

		CDX12Scene(CDX12Engine* engine,
			std::string  fileName);

		//--------------------------------------------------------------------------------------
		// Initialization
		//--------------------------------------------------------------------------------------

		void InitFrameDependentStuff();

		//--------------------------------------------------------------------------------------
		// Scene Render and Update
		//--------------------------------------------------------------------------------------

		// Returns the generated scene texture
		void RenderScene(float& frameTime) ;

		ImTextureID GetTextureSRV() ;

		void RenderSceneFromCamera(CCamera* camera) ;


		//--------------------------------------------------------------------------------------
		// DirectX12
		//--------------------------------------------------------------------------------------

		void Resize(UINT newX, UINT newY) ;

		~CDX12Scene();
		void UpdateScene(float& frameTime) ;
		void Save(std::string fileName) ;


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


		std::unique_ptr<CDX12RenderTarget> mSceneTexture;
		std::unique_ptr<CDX12DepthStencil> mDepthStencils[3];

		std::unique_ptr<CDX12DescriptorHeap> mDSVDescriptorHeap;
		std::unique_ptr<CDX12AmbientMap>     mAmbientMap;


	private:

		//--------------------------------------------------------------------------------------
		// Scene Data
		//--------------------------------------------------------------------------------------

		CDX12Engine* mEngine = nullptr;
		std::vector<ImTextureID> mShadowMaps;
		
		std::unique_ptr<CCamera>  mCamera;

		// Lock FPS to monitor refresh rate, which will typically set it to 60fps. Press 'p' to toggle to full fps
		bool         mLockFPS = true;
		UINT         mViewportX = 1920;
		UINT         mViewportY = 1080;
		int          mPcfSamples = 4;
		CDX12GameObject* mSelectedObj = nullptr;
		CWindow* mWindow = nullptr;
		std::string  mFileName;

		// Additional light information
		CVector3 gAmbientColour = { 0.03f,0.03f,0.04f }; // Background level of light (slightly bluish to match the far background, which is dark blue)
		ColourRGBA mBackgroundColor = { 0.3f,0.3f,0.4f,1.0f };
		float    gSpecularPower = 256; // Specular power //will be removed since it will be dependent on the material
	};
}
