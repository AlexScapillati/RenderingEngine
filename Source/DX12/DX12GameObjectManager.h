#pragma once

#include <vector>

#include "Objects/DX12GameObject.h"
#include "Objects/DX12Light.h"

class CDX12GameObjectManager
{
	public:
		CDX12GameObjectManager(CDX12Engine* engine);
		void AddObject(CDX12GameObject* obj);
		bool RenderAllObjects() const;
		void RenderFromSpotLights();
		void RenderFromPointLights();
		void RenderFromDirLights();
		void RenderFromAllLights() const;
		void UpdateObjects(float updateTime) const;
		void AddLight(CDX12Light* obj);


		std::vector<ID3D12Resource*>  mShadowMaps;
		std::vector<CDX12GameObject*> mObjects;
		std::vector<CDX12Light*>      mLights;

	private:
		CDX12Engine*     mEngine;
		CDX12GameObject* mSky;
		int              mMaxSize;
		int              mMaxShadowMaps;
};
