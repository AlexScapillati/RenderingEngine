#include "CGameObjectManager.h"

#include "../DX12/Objects/DX12Light.h"
#include "../DX12/Objects/DX12SpotLight.h"
#include "../DX12/Objects/DX12DirectionalLight.h"
#include "../DX12/Objects/DX12PointLight.h"
#include "../DX12/Objects/CDX12Sky.h"

CGameObjectManager::CGameObjectManager(DX12::CDX12Engine* engine)
{
	mEngine = engine;
	mMaxSize = 100;
	mMaxShadowMaps = 10;
}

void CGameObjectManager::AddObject(DX12::CDX12GameObject* obj)
{
	mObjects.push_back(obj);
}

void CGameObjectManager::AddLight(DX12::CDX12Light* obj)
{
	mLights.push_back(obj);
}

void CGameObjectManager::AddPointLight(DX12::CDX12PointLight* obj)
{
	mPointLights.push_back(obj);
}

void CGameObjectManager::AddSpotLight(DX12::CDX12SpotLight* obj)
{
	mSpotLights.push_back(obj);
}

void CGameObjectManager::AddDirLight(DX12::CDX12DirectionalLight* obj)
{
	mDirLights.push_back(obj);
}

void CGameObjectManager::AddSky(DX12::CDX12Sky* obj)
{
	//delete mSky;
	mSky = obj;
}

void CGameObjectManager::AddPlant(DX12::CDX12Plant* obj)
{
	mObjects.push_back(obj);
}

void CGameObjectManager::UpdateObjects(float updateTime) const
{
	for (const auto & o : mObjects)
	{
		o->Update(updateTime);
	}
}

void CGameObjectManager::RenderAllObjects() const
{
	// Render the objects
	for (const auto it : mObjects)
	{
		// Render the objects
		it->Render();
	}

	// Render the lights 
	for (const auto& it : mLights)
	{
		it->Render();
	}

	for (const auto& it : mSpotLights)
	{
		it->Render();
	}

	for (const auto& it : mDirLights)
	{
		it->Render();
	}

	for (const auto& it : mPointLights)
	{
		it->Render();
	}
}
