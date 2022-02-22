#include "CGameObjectManager.h"

#include "CGameObject.h"

CGameObjectManager::CGameObjectManager(IEngine* engine)
{
	mEngine = engine;
	mMaxSize = 100;
	mMaxShadowMaps = 10;
}

void CGameObjectManager::AddObject(CGameObject* obj)
{
	mObjects.push_back(obj);
}

void CGameObjectManager::AddLight(CLight* obj)
{
	mLights.push_back(obj);
}

void CGameObjectManager::AddPointLight(CPointLight* obj)
{
	mPointLights.push_back(obj);
}

void CGameObjectManager::AddSpotLight(CSpotLight* obj)
{
	mSpotLights.push_back(obj);
}

void CGameObjectManager::AddDirLight(CDirectionalLight* obj)
{
	mDirLights.push_back(obj);
}

void CGameObjectManager::AddSky(CSky* obj)
{
	if (mSky) delete obj;
	mSky = obj;
}

void CGameObjectManager::AddPlant(CPlant* obj)
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
	// Firstly render the sky (if any)
	if (mSky) mSky->Render();

	// Render the objects
	for (const auto it : mObjects)
	{
		// Render the objects
		it->Render();
	}

	// Render the lights 
	for (const auto it : mLights)
	{
		it->Render();
	}

	for (const auto it : mSpotLights)
	{
		it->Render();
	}

	for (const auto it : mDirLights)
	{
		it->Render();
	}

	for (const auto it : mPointLights)
	{
		it->Render();
	}
}
