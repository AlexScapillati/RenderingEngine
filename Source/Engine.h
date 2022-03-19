#pragma once

#include <wrl.h>
#include <string>
#include <memory>

#include "Math/CVector3.h"
#include "Utility/Timer.h"

// Forward declarations

class CGameObjectManager;
class CGui;
class CPlant;
class CSky;
class CPointLight;
class CDirectionalLight;
class CSpotLight;
class CLight;
class CGameObject;
class CScene;
class CWindow;

class IEngine
{
public:

	//*******************************
	//**** Virtual Functions 

	virtual bool Update() = 0;

	virtual void Resize(UINT x, UINT y) = 0;

	virtual void FinalizeFrame() = 0;

	virtual void CreateScene(std::string fileName = "") = 0;

	virtual CGameObject* CreateObject(
		const std::string& mesh,
		const std::string& name,
		const std::string& diffuseMap,
		CVector3           position = { 0,0,0 },
		CVector3           rotation = { 0,0,0 },
		float              scale = 1) = 0;

	virtual CSky* CreateSky(
		const std::string& mesh,
		const std::string& name,
		const std::string& diffuseMap,
		CVector3           position = { 0,0,0 },
		CVector3           rotation = { 0,0,0 },
		float              scale = 1) = 0;

	virtual CPlant* CreatePlant(
		const std::string& mesh,
		const std::string& name,
		CVector3           position = { 0,0,0 },
		CVector3           rotation = { 0,0,0 },
		float              scale = 1) = 0;

	virtual CGameObject* CreateObject(
		const std::string& dirPath,
		const std::string& name,
		CVector3           position = { 0,0,0 },
		CVector3           rotation = { 0,0,0 },
		float              scale = 1) = 0;

	virtual CLight* CreateLight(
		const std::string& mesh,
		const std::string& name,
		const std::string& diffuseMap,
		const CVector3& colour,
		const float& strength,
		CVector3           position = { 0,0,0 },
		CVector3           rotation = { 0,0,0 },
		float              scale = 1) = 0;

	virtual CSpotLight* CreateSpotLight(
		const std::string& mesh,
		const std::string& name,
		const std::string& diffuseMap,
		const CVector3& colour,
		const float& strength,
		CVector3           position = { 0,0,0 },
		CVector3           rotation = { 0,0,0 },
		float              scale = 1) = 0;

	virtual CDirectionalLight* CreateDirectionalLight(
		const std::string& mesh,
		const std::string& name,
		const std::string& diffuseMap,
		const CVector3& colour,
		const float& strength,
		CVector3           position = { 0,0,0 },
		CVector3           rotation = { 0,0,0 },
		float              scale = 1) = 0;

	virtual CPointLight* CreatePointLight(
		const std::string& mesh,
		const std::string& name,
		const std::string& diffuseMap,
		const CVector3& colour,
		const float& strength,
		CVector3           position = { 0,0,0 },
		CVector3           rotation = { 0,0,0 },
		float              scale = 1) = 0;

	//*******************************
	//**** Setters / Getters

	auto GetTimer() const { return mTimer; }

	auto GetWindow() const { return mWindow.get(); }

	auto GetScene() const { return mScene.get(); }

	auto& GetMediaFolder() const { return mMediaFolder; }

	auto& GetShaderFolder() const { return mShaderFolder; }

	auto GetObjManager() const { return mObjManager.get(); }

	virtual ~IEngine() = default;

protected:

	//*******************************
	//**** Data

	Timer mTimer;

	std::unique_ptr<CScene> mScene;

	std::unique_ptr<CGameObjectManager> mObjManager;

	std::unique_ptr<CGui> mGui;

	std::unique_ptr<CWindow> mWindow;

	std::string mMediaFolder;

	std::string mShaderFolder;

	std::string mPostProcessingFolder;
};
