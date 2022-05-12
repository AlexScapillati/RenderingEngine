#pragma once
#include <deque>

namespace DX12
{
	class CDX12Plant;
	class CDX12Light;
	class CDX12PointLight;
	class CDX12SpotLight;
	class CDX12DirectionalLight;
	class CDX12Sky;
	class CDX12GameObject;
	class CDX12Engine;
}


class CGameObjectManager
{
	
	public:

		CGameObjectManager()                                      = delete;
		CGameObjectManager(const CGameObjectManager&)             = delete;
		CGameObjectManager(const CGameObjectManager&&)            = delete;
		CGameObjectManager& operator=(const CGameObjectManager&)  = delete;
		CGameObjectManager& operator=(const CGameObjectManager&&) = delete;

		CGameObjectManager(DX12::CDX12Engine* engine);

		void AddObject		(DX12::CDX12GameObject* obj);
		void AddLight		(DX12::CDX12Light* obj);
		void AddPointLight	(DX12::CDX12PointLight* obj);
		void AddSpotLight	(DX12::CDX12SpotLight* obj);
		void AddDirLight	(DX12::CDX12DirectionalLight* obj);
		void AddSky			(DX12::CDX12Sky* obj);
		void AddPlant		(DX12::CDX12Plant* obj);

		void RenderAllObjects() const;
		void UpdateObjects(float updateTime) const;

		std::deque<DX12::CDX12GameObject*> mObjects {};
		std::deque<DX12::CDX12Light*> mLights {};
		std::deque<DX12::CDX12PointLight*> mPointLights {};
		std::deque<DX12::CDX12SpotLight*> mSpotLights {};
		std::deque<DX12::CDX12DirectionalLight*> mDirLights {};
		DX12::CDX12Sky*			 mSky = nullptr;

	private:
		DX12::CDX12Engine*	 mEngine;
		int              mMaxSize;
		int              mMaxShadowMaps;
	};


