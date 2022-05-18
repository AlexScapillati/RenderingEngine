
#include "LevelImporter.h"

#include <stdexcept>

#include "..\Common/Camera.h"

#include "CGameObjectManager.h"

#include "../DX12/DX12Engine.h"
#include "../DX12/DX12Scene.h"
#include "../DX12/Objects/CDX12Sky.h"
#include "../DX12/Objects/DX12Light.h"

#include <thread_pool.hpp>


DX12::CDX12Engine* mEngine;

bool CLevelImporter::LoadScene(const std::string& level, DX12::CDX12Engine* engine)
{
	mEngine = engine;

	tinyxml2::XMLDocument doc;

	if (doc.LoadFile(level.c_str()) != tinyxml2::XMLError::XML_SUCCESS)
	{
		throw std::runtime_error("Error opening file");
	}

	auto element = doc.FirstChildElement();

	while (element != nullptr)
	{
		std::string elementName = element->Name();
		if (elementName == "Scene")
		{
			try
			{
				ParseScene(element);
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(e.what());
			}
		}

		element = element->NextSiblingElement();
	}

	return true;
}

void  CLevelImporter::SaveScene(std::string& fileName /* ="" */)
{
	if (fileName.empty())
	{
		//save the scene in the project folder
		return;
	}

	tinyxml2::XMLDocument doc;

	const auto scene = doc.NewElement("Scene");

	doc.InsertFirstChild(scene);

	const auto def = scene->InsertNewChildElement("Default");

	auto defShaders = def->InsertNewChildElement("Shaders");

	const auto entities = scene->InsertNewChildElement("Entities");

	SaveObjects(entities);


	const auto ppEffects = scene->InsertNewChildElement("PostProcessingEffects");

	SavePostProcessingEffect(ppEffects);

	doc.InsertEndChild(scene);

	if (doc.SaveFile(fileName.c_str()) != tinyxml2::XMLError::XML_SUCCESS)
	{
		throw std::runtime_error("unable to save");
	}
}


void SavePositionRotationScale(tinyxml2::XMLElement*  obj,
                               DX12::CDX12GameObject* it)
{
	//save position, position and scale
	auto childEl = obj->InsertNewChildElement("Position");
	SaveVector3(it->Position(), childEl);
	childEl = obj->InsertNewChildElement("Rotation");
	SaveVector3(ToDegrees(it->Rotation()), childEl);
	childEl = obj->InsertNewChildElement("Scale");
	SaveVector3(it->Scale(), childEl);
}

void  SaveObjects(tinyxml2::XMLElement* el)
{
	//----------------------------------------------------
	//	Sky
	//----------------------------------------------------

	{
		auto sky = mEngine->GetObjManager()->mSky;

		const auto obj = el->InsertNewChildElement("Entity");

		if (auto plant = dynamic_cast<DX12::CDX12Plant*>(sky))
		{
			obj->SetAttribute("Type", "Plant");
		}
		else
		{
			obj->SetAttribute("Type", "GameObject");
		}

		obj->SetAttribute("Name", sky->Name().c_str());

		auto childEl = obj->InsertNewChildElement("Geometry");

		if (sky->IsPbr())
		{
			std::string id = sky->GetMeshes().front();

			childEl->SetAttribute("ID", id.c_str());
		}
		else
		{
			childEl->SetAttribute("Mesh", sky->GetMeshes().front().c_str());
			childEl->SetAttribute("Diffuse", sky->TextureFileName().c_str());
		}

		SavePositionRotationScale(obj, sky);
	}

	//----------------------------------------------------
	//	Game Objects
	//----------------------------------------------------

	for (const auto it : mEngine->GetObjManager()->mObjects)
	{
		const auto obj = el->InsertNewChildElement("Entity");

		if (auto plant = dynamic_cast<DX12::CDX12Plant*>(it))
		{
			obj->SetAttribute("Type", "Plant");
		}
		else
		{
			obj->SetAttribute("Type", "GameObject");
		}

		obj->SetAttribute("Name", it->Name().c_str());

		auto childEl = obj->InsertNewChildElement("Geometry");

		if (it->IsPbr())
		{
			std::string id = it->GetMeshes().front();

			childEl->SetAttribute("ID", id.c_str());
		}
		else
		{
			childEl->SetAttribute("Mesh",	it->GetMeshes().front().c_str());
			childEl->SetAttribute("Diffuse", it->TextureFileName().c_str());
		}

		SavePositionRotationScale(obj, it);
	}

	//----------------------------------------------------
	//	Simple Lights
	//----------------------------------------------------

	for (const auto it : mEngine->GetObjManager()->mLights)
	{
		const auto obj = el->InsertNewChildElement("Entity");
		obj->SetAttribute("Type", "Light");
		obj->SetAttribute("Name", it->Name().c_str());

		auto childEl = obj->InsertNewChildElement("Geometry");

		childEl->SetAttribute("Mesh", it->MeshFileNames().c_str());
		childEl->SetAttribute("Diffuse", it->TextureFileName().c_str());

		//save position, position and scale
		SavePositionRotationScale(obj, it);

		//save colour and strength
		childEl = obj->InsertNewChildElement("Colour");
		SaveVector3(it->GetColour(), childEl);
		childEl = obj->InsertNewChildElement("Strength");
		childEl->SetAttribute("S", it->GetStrength());
	}

	//----------------------------------------------------
	//	Spot Lights
	//----------------------------------------------------

	for (const auto it : mEngine->GetObjManager()->mSpotLights)
	{
		const auto obj = el->InsertNewChildElement("Entity");
		obj->SetAttribute("Type", "SpotLight");
		obj->SetAttribute("Name", it->Name().c_str());

		auto childEl = obj->InsertNewChildElement("Geometry");

		childEl->SetAttribute("Mesh", it->MeshFileNames().c_str());
		childEl->SetAttribute("Diffuse", it->TextureFileName().c_str());

		//save position, position and scale
		SavePositionRotationScale(obj, it);

		//save colour and strength
		childEl = obj->InsertNewChildElement("Colour");
		SaveVector3(it->GetColour(), childEl);
		childEl = obj->InsertNewChildElement("Strength");
		childEl->SetAttribute("S", it->GetStrength());
	}

	//----------------------------------------------------
	//	Directional Lights
	//----------------------------------------------------

	for (const auto it : mEngine->GetObjManager()->mDirLights)
	{
		const auto obj = el->InsertNewChildElement("Entity");
		obj->SetAttribute("Type", "DirectionalLight");
		obj->SetAttribute("Name", it->Name().c_str());

		auto childEl = obj->InsertNewChildElement("Geometry");

		childEl->SetAttribute("Mesh", it->MeshFileNames().c_str());
		childEl->SetAttribute("Diffuse", it->TextureFileName().c_str());

		//save rotation and scale
		SavePositionRotationScale(obj, it);

		//save colour and strength
		childEl = obj->InsertNewChildElement("Colour");
		SaveVector3(it->GetColour(), childEl);
		childEl = obj->InsertNewChildElement("Strength");
		childEl->SetAttribute("S", it->GetStrength());
	}

	//----------------------------------------------------
	//	Point Lights
	//----------------------------------------------------

	for (const auto it : mEngine->GetObjManager()->mPointLights)
	{
		const auto obj = el->InsertNewChildElement("Entity");
		obj->SetAttribute("Type", "PointLight");
		obj->SetAttribute("Name", it->Name().c_str());

		auto childEl = obj->InsertNewChildElement("Geometry");

		childEl->SetAttribute("Mesh", it->MeshFileNames().c_str());
		childEl->SetAttribute("Diffuse", it->TextureFileName().c_str());

		//save position, position and scale
		SavePositionRotationScale(obj, it);

		//save colour and strength
		childEl = obj->InsertNewChildElement("Colour");
		SaveVector3(it->GetColour(), childEl);
		childEl = obj->InsertNewChildElement("Strength");
		childEl->SetAttribute("S", it->GetStrength());
	}

	//----------------------------------------------------
	//	Camera
	//----------------------------------------------------

	const auto camera = mEngine->GetScene()->GetCamera();

	const auto obj = el->InsertNewChildElement("Entity");
	obj->SetAttribute("Type", "Camera");

	//save position, position
	auto childEl = obj->InsertNewChildElement("Position");
	SaveVector3(camera->Position(), childEl);
	childEl = obj->InsertNewChildElement("Rotation");
	SaveVector3(ToDegrees(camera->Rotation()), childEl);
}

void  SaveVector3(CVector3              v,
	tinyxml2::XMLElement* el)
{
	el->SetAttribute("X", v.x);
	el->SetAttribute("Y", v.y);
	el->SetAttribute("Z", v.z);
}

bool  ParseScene(tinyxml2::XMLElement* sceneEl)
{
	auto element = sceneEl->FirstChildElement();

	while (element != nullptr)
	{
		std::string elementName = element->Name();

		if (elementName == "Entities")
		{
			try
			{
				ParseEntities(element);
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(e.what());
			}
		}
		else if (elementName == "PostProcessingEffects")
		{
			ParsePostProcessingEffects(element);
		}
		element = element->NextSiblingElement();
	}

	return true;
}

CVector3 LoadVector3(tinyxml2::XMLElement* el)
{
	return {
		float(el->FindAttribute("X")->DoubleValue()),
		float(el->FindAttribute("Y")->DoubleValue()),
		float(el->FindAttribute("Z")->DoubleValue())
	};
}

void  LoadObject(tinyxml2::XMLElement* currEntity)
{
	std::string ID;
	std::string mesh;
	std::string name;
	std::string diffuse;

	CVector3 pos = { 0,0,0 };
	CVector3 rot = { 0,0,0 };
	auto     scale = 1.0f;

	const auto entityNameAttr = currEntity->FindAttribute("Name");
	if (entityNameAttr)
		name = entityNameAttr->Value();

	const auto geometry = currEntity->FirstChildElement("Geometry");

	if (geometry)
	{
		const auto idAttr = geometry->FindAttribute("ID");
		if (idAttr) ID = idAttr->Value();

		const auto meshAttr = geometry->FindAttribute("Mesh");
		if (meshAttr) mesh = meshAttr->Value();

		const auto diffuseAttr = geometry->FindAttribute("Diffuse");
		if (diffuseAttr) diffuse = diffuseAttr->Value();
	}

	if (const auto positionEl = currEntity->FirstChildElement("Position"))
	{
		pos = LoadVector3(positionEl);
	}

	if (const auto rotationEl = currEntity->FirstChildElement("Rotation"))
	{
		rot = ToRadians(LoadVector3(rotationEl));
	}

	if (const auto scaleEl = currEntity->FirstChildElement("Scale"))
	{
		scale = scaleEl->FindAttribute("X")->FloatValue();
	}

	const auto ambientMapEl = currEntity->FirstChildElement("AmbientMap");

	bool enabled = false;
	int  size = 1;

	if (ambientMapEl)
	{
		enabled = ambientMapEl->FindAttribute("Enabled")->BoolValue();
		size = ambientMapEl->FindAttribute("Size")->IntValue();
	}

	// Create objects
	DX12::CDX12GameObject* obj = nullptr;

	if(diffuse.empty() && ID.empty())
	{
		obj = mEngine->CreateObject(mesh, name, pos, rot, scale);
	}
	else if (ID.empty())
	{
		obj = mEngine->CreateObject(mesh, name, diffuse, pos, rot, scale);
	}
	else
	{
		obj = mEngine->CreateObject(ID, name, pos, rot, scale);
	}


	// Set ambient map values
	//obj->AmbientMapEnabled() = enabled;
	//obj->AmbientMap()->SetSize(size);

}

void  LoadPointLight(tinyxml2::XMLElement* currEntity)
{
	std::string mesh;
	std::string name;
	std::string diffuse;

	CVector3 colour = { 0,0,0 };
	float    strength = 0;
	CVector3 pos = { 0,0,0 };
	CVector3 rot = { 0,0,0 };
	auto     scale = 1.0f;

	const auto entityNameAttr = currEntity->FindAttribute("Name");
	if (entityNameAttr) name = entityNameAttr->Value();

	const auto geometry = currEntity->FirstChildElement("Geometry");
	if (geometry)
	{
		const auto meshAttr = geometry->FindAttribute("Mesh");
		if (meshAttr) mesh = meshAttr->Value();

		const auto diffuseAttr = geometry->FindAttribute("Diffuse");
		if (diffuseAttr) diffuse = diffuseAttr->Value();
	}

	const auto positionEl = currEntity->FirstChildElement("Position");
	if (positionEl)
	{
		pos = LoadVector3(positionEl);
	}

	const auto rotationEl = currEntity->FirstChildElement("Rotation");
	if (rotationEl)
	{
		rot = ToRadians(LoadVector3(rotationEl));
	}

	const auto scaleEl = currEntity->FirstChildElement("Scale");
	if (scaleEl)
	{
		scale = scaleEl->FindAttribute("X")->FloatValue();
	}

	const auto strengthEl = currEntity->FirstChildElement("Strength");
	if (strengthEl)
	{
		strength = strengthEl->FindAttribute("S")->FloatValue();
	}

	const auto colourEl = currEntity->FirstChildElement("Colour");
	if (colourEl)
	{
		colour = LoadVector3(colourEl);
	}

	auto obj = mEngine->CreatePointLight(mesh, name, diffuse, colour, strength, pos, rot, scale);
}

void  LoadLight(tinyxml2::XMLElement* currEntity)
{
	std::string mesh;
	std::string name;
	std::string diffuse;

	CVector3 colour = { 0,0,0 };
	float    strength = 0;
	CVector3 pos = { 0,0,0 };
	CVector3 rot = { 0,0,0 };
	auto     scale = 1.0f;

	const auto entityNameAttr = currEntity->FindAttribute("Name");
	if (entityNameAttr) name = entityNameAttr->Value();

	const auto geometry = currEntity->FirstChildElement("Geometry");
	if (geometry)
	{
		const auto meshAttr = geometry->FindAttribute("Mesh");
		if (meshAttr) mesh = meshAttr->Value();

		const auto diffuseAttr = geometry->FindAttribute("Diffuse");
		if (diffuseAttr) diffuse = diffuseAttr->Value();
	}

	const auto positionEl = currEntity->FirstChildElement("Position");
	if (positionEl)
	{
		pos = LoadVector3(positionEl);
	}

	const auto rotationEl = currEntity->FirstChildElement("Rotation");
	if (rotationEl)
	{
		rot = LoadVector3(rotationEl);
	}

	const auto scaleEl = currEntity->FirstChildElement("Scale");
	if (scaleEl)
	{
		scale = scaleEl->FindAttribute("X")->FloatValue();
	}

	const auto strengthEl = currEntity->FirstChildElement("Strength");
	if (strengthEl)
	{
		strength = strengthEl->FindAttribute("S")->FloatValue();
	}

	const auto colourEl = currEntity->FirstChildElement("Colour");
	if (colourEl)
	{
		colour = LoadVector3(colourEl);
	}

	auto obj = mEngine->CreateLight(mesh, name, diffuse, colour, strength, pos, rot, scale);
}


void  LoadSpotLight(tinyxml2::XMLElement* currEntity)
{
	std::string mesh;
	std::string name;
	std::string diffuse;

	CVector3 colour = { 0,0,0 };
	float    strength = 0;
	CVector3 pos = { 0,0,0 };
	CVector3 rot = { 0,0,0 };
	auto     scale = 1.0f;

	const auto entityNameAttr = currEntity->FindAttribute("Name");
	if (entityNameAttr) name = entityNameAttr->Value();

	const auto geometry = currEntity->FirstChildElement("Geometry");
	if (geometry)
	{
		const auto meshAttr = geometry->FindAttribute("Mesh");
		if (meshAttr) mesh = meshAttr->Value();

		const auto diffuseAttr = geometry->FindAttribute("Diffuse");
		if (diffuseAttr) diffuse = diffuseAttr->Value();
	}

	const auto positionEl = currEntity->FirstChildElement("Position");
	if (positionEl)
	{
		pos = LoadVector3(positionEl);
	}

	const auto rotationEl = currEntity->FirstChildElement("Rotation");
	if (rotationEl)
	{
		rot = LoadVector3(rotationEl);
	}

	const auto scaleEl = currEntity->FirstChildElement("Scale");
	if (scaleEl)
	{
		scale = scaleEl->FindAttribute("X")->FloatValue();
	}

	const auto strengthEl = currEntity->FirstChildElement("Strength");
	if (strengthEl)
	{
		strength = strengthEl->FindAttribute("S")->FloatValue();
	}

	const auto colourEl = currEntity->FirstChildElement("Colour");
	if (colourEl)
	{
		colour = LoadVector3(colourEl);
	}

	auto obj = mEngine->CreateSpotLight(mesh, name, diffuse, colour, strength, pos, rot, scale);
}


void  LoadDirLight(tinyxml2::XMLElement* currEntity)
{
	std::string mesh;
	std::string name;
	std::string diffuse;

	CVector3 colour = { 0,0,0 };
	float    strength = 0;
	CVector3 pos = { 0,0,0 };
	CVector3 rot = { 0,0,0 };
	auto     scale = 1.0f;

	const auto entityNameAttr = currEntity->FindAttribute("Name");
	if (entityNameAttr) name = entityNameAttr->Value();

	const auto geometry = currEntity->FirstChildElement("Geometry");
	if (geometry)
	{
		const auto meshAttr = geometry->FindAttribute("Mesh");
		if (meshAttr) mesh = meshAttr->Value();

		const auto diffuseAttr = geometry->FindAttribute("Diffuse");
		if (diffuseAttr) diffuse = diffuseAttr->Value();
	}

	const auto positionEl = currEntity->FirstChildElement("Position");
	if (positionEl)
	{
		pos = LoadVector3(positionEl);
	}

	const auto rotationEl = currEntity->FirstChildElement("Rotation");
	if (rotationEl)
	{
		rot = LoadVector3(rotationEl);
	}

	const auto scaleEl = currEntity->FirstChildElement("Scale");
	if (scaleEl)
	{
		scale = scaleEl->FindAttribute("X")->FloatValue();
	}

	const auto strengthEl = currEntity->FirstChildElement("Strength");
	if (strengthEl)
	{
		strength = strengthEl->FindAttribute("S")->FloatValue();
	}

	const auto colourEl = currEntity->FirstChildElement("Colour");
	if (colourEl)
	{
		colour = LoadVector3(colourEl);
	}

	auto obj = mEngine->CreateDirectionalLight(mesh, name, diffuse, colour, strength, pos, rot, scale);
}


void  LoadSky(tinyxml2::XMLElement* currEntity)
{
	std::string mesh;
	std::string name;
	std::string diffuse;

	CVector3 pos = { 0,0,0 };
	CVector3 rot = { 0,0,0 };
	auto     scale = 1.0f;

	const auto entityNameAttr = currEntity->FindAttribute("Name");
	if (entityNameAttr)
		name = entityNameAttr->Value();

	const auto geometry = currEntity->FirstChildElement("Geometry");

	if (geometry)
	{
		const auto meshAttr = geometry->FindAttribute("Mesh");
		if (meshAttr) mesh = meshAttr->Value();

		const auto diffuseAttr = geometry->FindAttribute("Diffuse");
		if (diffuseAttr) diffuse = diffuseAttr->Value();
	}

	const auto positionEl = currEntity->FirstChildElement("Position");
	if (positionEl)
	{
		pos = LoadVector3(positionEl);
	}

	const auto rotationEl = currEntity->FirstChildElement("Rotation");
	if (rotationEl)
	{
		rot = ToRadians(LoadVector3(rotationEl));
	}

	const auto scaleEl = currEntity->FirstChildElement("Scale");
	if (scaleEl)
	{
		scale = scaleEl->FindAttribute("X")->FloatValue();
	}

	// No ambient map for the sky object

	mEngine->CreateSky(mesh, name, diffuse, pos, rot, scale);
}

void  LoadCamera(tinyxml2::XMLElement* currEntity)
{
	const auto  FOV = PI / 3;
	const auto  aspectRatio = 1.333333373f;
	const auto  nearClip = 0.100000015f;
	const auto  farClip = 10000.0f;

	CVector3 pos = { 0,0,0 };
	CVector3 rot = { 0,0,0 };

	const auto positionEl = currEntity->FirstChildElement("Position");
	if (positionEl)
	{
		pos = LoadVector3(positionEl);
	}

	const auto rotationEl = currEntity->FirstChildElement("Rotation");
	if (rotationEl)
	{
		rot = ToRadians(LoadVector3(rotationEl));
	}

	auto c = new CCamera(pos, rot, FOV, aspectRatio, nearClip, farClip);
	mEngine->GetScene()->SetCamera(c);
}

void  LoadPlant(tinyxml2::XMLElement* currEntity)
{
	std::string ID;
	std::string name;
	std::string mesh;

	CVector3 pos = { 0,0,0 };
	CVector3 rot = { 0,0,0 };
	auto     scale = 1.0f;

	const auto entityNameAttr = currEntity->FindAttribute("Name");
	if (entityNameAttr)
		name = entityNameAttr->Value();

	if (const auto geometry = currEntity->FirstChildElement("Geometry"))
	{
		const auto idAttr = geometry->FindAttribute("ID");
		if (idAttr) ID = idAttr->Value();
		else
		{
			const auto meshAttr = geometry->FindAttribute("Mesh");
			if (meshAttr) mesh = meshAttr->Value();

		}
	}

	if (const auto positionEl = currEntity->FirstChildElement("Position"))
	{
		pos = LoadVector3(positionEl);
	}

	if (const auto rotationEl = currEntity->FirstChildElement("Rotation"))
	{
		rot = ToRadians(LoadVector3(rotationEl));
	}

	if (const auto scaleEl = currEntity->FirstChildElement("Scale"))
	{
		scale = scaleEl->FindAttribute("X")->FloatValue();
	}

	try
	{
		DX12::CDX12Plant* obj;
		if(!ID.empty())
			obj = mEngine->CreatePlant(ID, name, pos, rot, scale);
		else
			obj = mEngine->CreatePlant(mesh, name, pos, rot, scale);


		mEngine->GetObjManager()->AddPlant(obj);
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(std::string(e.what()) + " of object " + name);
	}
}

void  SavePostProcessingEffect(tinyxml2::XMLElement* curr)
{
}

void  ParsePostProcessingEffects(tinyxml2::XMLElement* curr)
{

}

bool  ParseEntities(tinyxml2::XMLElement* entitiesEl)
{
	auto currEntity = entitiesEl->FirstChildElement();

	thread_pool TPool(2);

	while (currEntity)
	{
		std::string entityName = currEntity->Name();

		if (entityName == "Entity")
		{
			const auto type = currEntity->FindAttribute("Type");

			if (type)
			{
				std::string typeValue = type->Value();

				if (typeValue == "GameObject")
				{
					TPool.push_task(LoadObject, currEntity);
				}
				else if (typeValue == "Light")
				{
					TPool.push_task(LoadLight,currEntity);
				}
				else if (typeValue == "PointLight")
				{
					TPool.push_task(LoadPointLight,currEntity);
				}
				else if (typeValue == "DirectionalLight")
				{
					TPool.push_task(LoadDirLight,currEntity);
				}
				else if (typeValue == "SpotLight")
				{
					TPool.push_task(LoadSpotLight,currEntity);
				}
				else if (typeValue == "Sky")
				{
					TPool.push_task(LoadSky,currEntity);
				}
				else if (typeValue == "Plant")
				{
					TPool.push_task(LoadPlant,currEntity);
				}
				else if (typeValue == "Camera")
				{
					//LoadCamera(currEntity);
				}
			}
		}
		currEntity = currEntity->NextSiblingElement();
	}

	TPool.wait_for_tasks();

	return true;
}
