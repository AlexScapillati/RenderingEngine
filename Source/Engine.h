#pragma once

#include <wrl.h>
#include <string>
#include <memory>

#include "Common/CScene.h"
#include "Math/CVector3.h"
#include "Utility/Timer.h"

// Forward declarations

class CGui;
class CPlant;
class CSky;
class CPointLight;
class CDirectionalLight;
class CSpotLight;
class CLight;
class CGameObject;

class CWindow;

template <typename Impl>
class IEngine
{
<<<<<<< Updated upstream
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

	auto GetTimer() const
	{
		return mTimer;
	}

	auto GetWindow() const
	{
		return mWindow.get();
	}

	auto GetScene() const
	{
		return mScene.get();
	}


	auto& GetMediaFolder()
	{
		return mMediaFolder;
	}

	virtual ~IEngine() = default;

protected:

	//*******************************
	//**** Data

	Timer mTimer;

	std::unique_ptr<CScene> mScene;

	std::unique_ptr<CGui> mGui;

	std::unique_ptr<CWindow> mWindow;

	std::string mMediaFolder;

	std::string mShaderFolder;

	std::string mPostProcessingFolder;
=======
	public:
		//*******************************
		//****  Functions 

		bool Update() { return impl()->UpdateImpl(); };

		void Resize(UINT x, UINT y) { impl()->ResizeImpl(); }

		void FinalizeFrame() { impl()->FinalizeFrameImpl(); }

		void CreateScene(std::string fileName = "") { impl()->CreateSceneImpl(fileName); };

		CScene<Impl>* GetScene() const { return impl()->GetSceneImpl(); }

		CGameObject* CreateObject(const std::string& mesh,
								  const std::string& name,
								  const std::string& diffuseMap,
								  CVector3           position = { 0,0,0 },
								  CVector3           rotation = { 0,0,0 },
								  float              scale    = 1)
		{
			return impl()->CreateObjectImpl(mesh, name, diffuseMap, position, rotation, scale);
		};

		CSky* CreateSky(const std::string& mesh,
						const std::string& name,
						const std::string& diffuseMap,
						CVector3           position = { 0,0,0 },
						CVector3           rotation = { 0,0,0 },
						float              scale    = 1)
		{
			return impl()->CreateSkyImpl(mesh, name, diffuseMap, position, rotation, scale);
		};

		CPlant* CreatePlant(const std::string& mesh,
							const std::string& name,
							CVector3           position = { 0,0,0 },
							CVector3           rotation = { 0,0,0 },
							float              scale    = 1)
		{
			return impl()->CreatePlantImpl(mesh, name, position, rotation, scale);
		};

		CGameObject* CreateObject(const std::string& dirPath,
								  const std::string& name,
								  CVector3           position = { 0,0,0 },
								  CVector3           rotation = { 0,0,0 },
								  float              scale    = 1)
		{
			return impl()->CreateObjectImpl(dirPath, name, position, rotation, scale);
		};

		CLight* CreateLight(const std::string& mesh,
							const std::string& name,
							const std::string& diffuseMap,
							const CVector3&    colour,
							const float&       strength,
							CVector3           position = { 0,0,0 },
							CVector3           rotation = { 0,0,0 },
							float              scale    = 1)
		{
			return impl()->CreateLightImpl(mesh, name, diffuseMap, colour, strength, position, rotation, scale);
		};

		CSpotLight* CreateSpotLight(const std::string& mesh,
									const std::string& name,
									const std::string& diffuseMap,
									const CVector3&    colour,
									const float&       strength,
									CVector3           position = { 0,0,0 },
									CVector3           rotation = { 0,0,0 },
									float              scale    = 1)
		{
			return impl()->CreateSpotLightImpl(mesh, name, diffuseMap, colour, strength, position, rotation, scale);
		};

		CDirectionalLight* CreateDirectionalLight(const std::string& mesh,
												  const std::string& name,
												  const std::string& diffuseMap,
												  const CVector3&    colour,
												  const float&       strength,
												  CVector3           position = { 0,0,0 },
												  CVector3           rotation = { 0,0,0 },
												  float              scale    = 1)
		{
			return impl()->CreateDirectionalLightImpl(mesh, name, diffuseMap, colour, strength, position, rotation, scale);
		};

		CPointLight* CreatePointLight(const std::string& mesh,
									  const std::string& name,
									  const std::string& diffuseMap,
									  const CVector3&    colour,
									  const float&       strength,
									  CVector3           position = { 0,0,0 },
									  CVector3           rotation = { 0,0,0 },
									  float              scale    = 1)
		{
			return impl()->CreatePointLightImpl(mesh, name, diffuseMap, colour, strength, position, rotation, scale);
		};

		//*******************************
		//**** Setters / Getters

		auto GetTimer() const { return mTimer; }

		auto GetWindow() const { return mWindow.get(); }


		auto& GetMediaFolder() const { return mMediaFolder; }

		auto& GetShaderFolder() const { return mShaderFolder; }

		auto GetObjManager() const { return mObjManager.get(); }

		~IEngine() = default;

	protected:
		Impl* impl() { return static_cast<Impl*>(this); }

		//*******************************
		//**** Data

		Timer mTimer;

		std::unique_ptr<CGameObjectManager> mObjManager;

		std::unique_ptr<CGui> mGui;

		std::unique_ptr<CWindow> mWindow;

		std::string mMediaFolder;

		std::string mShaderFolder;

		std::string mPostProcessingFolder;
>>>>>>> Stashed changes
};
