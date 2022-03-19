#pragma once

#include "..\External/tinyxml2/tinyxml2.h"
#include <string>

template<typename Impl>
class IEngine;

class CGameObject;
class CVector3;

class CLevelImporter
{
public:

	//--------------------------------------------------------------------------------------
	// Scene Parser
	//--------------------------------------------------------------------------------------

	bool LoadScene(const std::string& level );

	void SaveScene(std::string& fileName);

	template <typename Impl>
	CLevelImporter(IEngine<Impl>* engine) : mEngine(engine) {}

private:

	template <typename Impl>
	IEngine<Impl>* mEngine;

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
};
