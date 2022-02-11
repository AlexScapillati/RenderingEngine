#include "DX12GameObjectManager.h"
#include "Objects/CDX12Sky.h"

CDX12GameObjectManager::CDX12GameObjectManager(CDX12Engine* engine)
{
	mSky               = nullptr;
	mEngine            = engine;
	mMaxSize           = 100;
	mMaxShadowMaps	   = 10;
}

void CDX12GameObjectManager::AddObject(CDX12GameObject* obj)
{
	if (mObjects.size() < mMaxSize)
	{
		// Try to cast it to the sky object // useful for the ambient map
		if (!mSky)
		{
			mSky = dynamic_cast<CDX12Sky*>(obj);
		}

		mObjects.push_back(obj);
	}
	else
	{
		throw std::exception("Not enough space to store more objects");
	}
}

bool CDX12GameObjectManager::RenderAllObjects() const
{
	for(const auto& o : mObjects)
	{
		o->Render();
	}
	return true;
}

void CDX12GameObjectManager::RenderFromSpotLights()
{
}

void CDX12GameObjectManager::RenderFromPointLights()
{
}

void CDX12GameObjectManager::RenderFromDirLights()
{
}

void CDX12GameObjectManager::RenderFromAllLights() const
{
	for(const auto& l : mLights)
	{
		l->Render();
	}
}

void CDX12GameObjectManager::UpdateObjects(float updateTime) const
{
	for (const auto& o : mObjects)
	{
		o->Update(updateTime);
	}
}

void CDX12GameObjectManager::AddLight(CDX12Light* obj)
{
	mLights.push_back(obj);
}
