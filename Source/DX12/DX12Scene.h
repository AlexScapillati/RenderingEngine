#pragma once

#include "DX12Engine.h"
#include "../Common/Camera.h"
#include "DX12GameObjectManager.h"

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

	ImTextureID GetTextureSrv() const;

	void RenderSceneFromCamera(CCamera* camera) const;

	//--------------------------------------------------------------------------------------
	// DirectX
	//--------------------------------------------------------------------------------------

	UINT mViewportX;
	UINT mViewportY;

	std::unique_ptr<CDX12RenderTarget> mSceneTexture;

	//--------------------------------------------------------------------------------------
	// Public Variables TODO REMOVE BIG NONO
	//--------------------------------------------------------------------------------------

	std::unique_ptr<CDX12GameObjectManager> mObjManager;
	std::unique_ptr<CCamera> mCamera;

	//--------------------------------------------------------------------------------------
	// Public Functions
	//--------------------------------------------------------------------------------------

	void Save(std::string fileName = "");
	void Resize(UINT newX, UINT newY);
	void PostProcessingPass();
	void RenderToDepthMap();
	void DisplayPostProcessingEffects(); // TODO: Remove


	//--------------------------------------------------------------------------------------
	// Getters 
	//--------------------------------------------------------------------------------------


	CDX12GameObjectManager* GetObjectManager() const;
	CVector2 GetViewportSize() const;
	UINT     GetViewportX() const;
	UINT     GetViewportY() const;
	CCamera* GetCamera() const;
	bool&    GetLockFps();


private:
	//--------------------------------------------------------------------------------------
	// Scene Data
	//--------------------------------------------------------------------------------------


	CDX12Engine* mEngine = nullptr;
	CWindow* mWindow = nullptr;

	std::string mFileName;

	// Lock FPS to monitor refresh rate, which will typically set it to 60fps. Press 'p' to toggle to full fps
	bool mLockFPS = true;

	// Additional light information
	CVector3 gAmbientColour;

	CDX12GameObject* mSelectedObj;
};
