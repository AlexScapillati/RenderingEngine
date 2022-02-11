#pragma once


#include "../tinyxml2/tinyxml2.h"
#include <string>

class CDX12GameObject;
class CDX12Scene;
class CDX12Engine;
class CVector3;

class CDX12Importer
{
public:

	//--------------------------------------------------------------------------------------
	// Scene Parser
	//--------------------------------------------------------------------------------------

	bool LoadScene(const std::string& level, CDX12Scene* scene);

	void SaveScene(std::string& fileName, CDX12Scene* ptrScene);

	CDX12Importer(CDX12Engine* engine);

private:

	CDX12Engine* mEngine;

	void ParsePostProcessingEffects(tinyxml2::XMLElement* curr, CDX12Scene* scene);

	void SavePostProcessingEffect(tinyxml2::XMLElement* curr, CDX12Scene* scene);

	void SavePositionRotationScale(tinyxml2::XMLElement* obj, CDX12GameObject* it);

	void SaveObjects(tinyxml2::XMLElement* el, CDX12Scene* ptrScene);

	void SaveVector3(CVector3 v, tinyxml2::XMLElement* el);

	CVector3 LoadVector3(tinyxml2::XMLElement* el);

	bool ParseScene(tinyxml2::XMLElement* sceneEl, CDX12Scene* scene);

	void LoadObject(tinyxml2::XMLElement* currEntity, CDX12Scene* scene);
	
	void LoadLight(tinyxml2::XMLElement* currEntity, CDX12Scene* scene);
	
	void LoadCamera(tinyxml2::XMLElement* currEntity, CDX12Scene* scene);
	
	bool ParseEntities(tinyxml2::XMLElement* entitiesEl, CDX12Scene* scene);
};
