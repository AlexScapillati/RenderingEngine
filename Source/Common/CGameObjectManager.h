#pragma once
#include <deque>

class CGameObject;
class CPlant;
class CSky;
class CDirectionalLight;
class CSpotLight;
class CPointLight;
class CLight;

template <typename Impl>
class IEngine<Impl>;

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
<<<<<<< HEAD
<<<<<<< HEAD

=======
		IEngine*		 mEngine;
		CSky*			 mSky = nullptr;
>>>>>>> parent of 5f2c2d1 (Working on IBL - DX12)
=======
		IEngine*		 mEngine;
>>>>>>> parent of 100d753 (Merge pull request #3 from AlexScapillati/TryingPolymorphism)
		int              mMaxSize;
		int              mMaxShadowMaps;
	};


