#pragma once
#include <deque>

class CGameObject;
class CPlant;
class CSky;
class CDirectionalLight;
class CSpotLight;
class CPointLight;
class CLight;
class IEngine;

class CGameObjectManager
{
	
	public:

		CGameObjectManager()                                      = delete;
		CGameObjectManager(const CGameObjectManager&)             = delete;
		CGameObjectManager(const CGameObjectManager&&)            = delete;
		CGameObjectManager& operator=(const CGameObjectManager&)  = delete;
		CGameObjectManager& operator=(const CGameObjectManager&&) = delete;

		CGameObjectManager(IEngine* engine);

		void AddObject		(CGameObject* obj);
		void AddLight		(CLight* obj);
		void AddPointLight	(CPointLight* obj);
		void AddSpotLight	(CSpotLight* obj);
		void AddDirLight	(CDirectionalLight* obj);
		void AddSky			(CSky* obj);
		void AddPlant		(CPlant* obj);

		void RenderAllObjects() const;
		void UpdateObjects(float updateTime) const;

		std::deque<CGameObject*> mObjects {};
		std::deque<CLight*> mLights {};
		std::deque<CPointLight*> mPointLights {};
		std::deque<CSpotLight*> mSpotLights {};
		std::deque<CDirectionalLight*> mDirLights {};

	private:
		IEngine*		 mEngine;
		CSky*			 mSky = nullptr;
		int              mMaxSize;
		int              mMaxShadowMaps;
	};


