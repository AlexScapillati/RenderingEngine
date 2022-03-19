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
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD

=======
		IEngine*		 mEngine;
		CSky*			 mSky = nullptr;
>>>>>>> parent of 5f2c2d1 (Working on IBL - DX12)
=======
		IEngine*		 mEngine;
>>>>>>> parent of 100d753 (Merge pull request #3 from AlexScapillati/TryingPolymorphism)
=======
		IEngine*		 mEngine;
<<<<<<< HEAD
<<<<<<< HEAD
>>>>>>> parent of b0bd427 (Up)
=======
		CSky*			 mSky;
>>>>>>> parent of 7bb1619 (Merge branch 'main' into TryingPolymorphism)
=======
		CSky*			 mSky = nullptr;
>>>>>>> parent of 5f2c2d1 (Working on IBL - DX12)
		int              mMaxSize;
		int              mMaxShadowMaps;
	};


