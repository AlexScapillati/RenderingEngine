#pragma once

#include "..\External/tinyxml2/tinyxml2.h"
#include <string>

class CGameObject;
class IEngine;
class CVector3;
class CScene;

class CLevelImporter
{
public:

	//--------------------------------------------------------------------------------------
	// Scene Parser
	//--------------------------------------------------------------------------------------

	bool LoadScene(const std::string& level, CScene* scene);

	void SaveScene(std::string& fileName, CScene* ptrScene);

	CLevelImporter(IEngine* engine);

private:
	
	IEngine* mEngine;

	void ParsePostProcessingEffects(tinyxml2::XMLElement* curr);
	
	void SavePostProcessingEffect(tinyxml2::XMLElement* curr);

	void SavePositionRotationScale(tinyxml2::XMLElement* obj, CGameObject* it);

	void SaveObjects(tinyxml2::XMLElement* el, CScene* ptrScene);

	void SaveVector3(CVector3 v, tinyxml2::XMLElement* el);

	bool ParseScene(tinyxml2::XMLElement* sceneEl, CScene* scene);

	void LoadObject(tinyxml2::XMLElement* currEntity, CScene* scene);

	void LoadPointLight(tinyxml2::XMLElement* currEntity, CScene* scene);

	void LoadLight(tinyxml2::XMLElement* currEntity, CScene* scene);

	void LoadSpotLight(tinyxml2::XMLElement* currEntity, CScene* scene);

	void LoadDirLight(tinyxml2::XMLElement* currEntity, CScene* scene);

	void LoadSky(tinyxml2::XMLElement* currEntity, CScene* scene);

	void LoadCamera(tinyxml2::XMLElement* currEntity, CScene* scene);

	void LoadPlant(tinyxml2::XMLElement* currEntity, CScene* scene);

	bool ParseEntities(tinyxml2::XMLElement* entitiesEl, CScene* scene);
};
