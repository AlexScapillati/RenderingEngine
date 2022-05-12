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

	static bool LoadScene(const std::string& level, IEngine* engine);

	static void SaveScene(std::string& fileName);

};

	void ParsePostProcessingEffects(tinyxml2::XMLElement* curr);
	
	void SavePostProcessingEffect(tinyxml2::XMLElement* curr);

	void SavePositionRotationScale(tinyxml2::XMLElement* obj, CGameObject* it);

	void SaveObjects(tinyxml2::XMLElement* el);

	void SaveVector3(CVector3 v, tinyxml2::XMLElement* el);

	bool ParseScene(tinyxml2::XMLElement* sceneEl);

	void LoadObject(tinyxml2::XMLElement* currEntity);

	void LoadPointLight(tinyxml2::XMLElement* currEntity);

	void LoadLight(tinyxml2::XMLElement* currEntity);

	void LoadSpotLight(tinyxml2::XMLElement* currEntity);

	void LoadDirLight(tinyxml2::XMLElement* currEntity);

	void LoadSky(tinyxml2::XMLElement* currEntity);

	void LoadCamera(tinyxml2::XMLElement* currEntity);

	void LoadPlant(tinyxml2::XMLElement* currEntity);

	bool ParseEntities(tinyxml2::XMLElement* entitiesEl);