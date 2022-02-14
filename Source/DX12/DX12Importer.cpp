#include "DX12Importer.h"

#include <stdexcept>

#include "CDX12Material.h"
#include "DX12GameObjectManager.h"
#include "DX12Mesh.h"
#include "DX12Scene.h"
#include "Objects/DX12GameObject.h"
#include "../Math/CVector3.h"
#include "Objects/DX12Light.h"
#include "../Common/Camera.h"

CVector3 CDX12Importer::LoadVector3(tinyxml2::XMLElement* el)
{
	return { float(el->FindAttribute("X")->DoubleValue()),
			float(el->FindAttribute("Y")->DoubleValue()),
			float(el->FindAttribute("Z")->DoubleValue()) };
}

bool CDX12Importer::LoadScene(const std::string& level, CDX12Scene* scene)
{
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
				ParseScene(element, scene);
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

void CDX12Importer::SaveScene(std::string& fileName, CDX12Scene* ptrScene)
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

	SaveObjects(entities, ptrScene);

	const auto ppEffects = scene->InsertNewChildElement("PostProcessingEffects");

	SavePostProcessingEffect(ppEffects, ptrScene);

	doc.InsertEndChild(scene);

	if (doc.SaveFile(fileName.c_str()) != tinyxml2::XMLError::XML_SUCCESS)
	{
		throw std::runtime_error("unable to save");
	}
}

CDX12Importer::CDX12Importer(CDX12Engine* engine)
{
	mEngine = engine;
}

void CDX12Importer::ParsePostProcessingEffects(tinyxml2::XMLElement* curr, CDX12Scene* scene)
{
}

void CDX12Importer::SavePostProcessingEffect(tinyxml2::XMLElement* curr, CDX12Scene* scene)
{
}

void CDX12Importer::SavePositionRotationScale(tinyxml2::XMLElement* obj, CDX12GameObject* it)
{
	//save position, position and scale
	auto childEl = obj->InsertNewChildElement("Position");
	SaveVector3(it->Position(), childEl);
	childEl = obj->InsertNewChildElement("Rotation");
	SaveVector3(ToDegrees(it->Rotation()), childEl);
	childEl = obj->InsertNewChildElement("Scale");
	SaveVector3(it->Scale(), childEl);
}

void CDX12Importer::SaveObjects(tinyxml2::XMLElement* el, CDX12Scene* ptrScene)
{
	//----------------------------------------------------
	//	Game Objects
	//----------------------------------------------------

	for (const auto it : ptrScene->GetObjectManager()->mObjects)
	{
		const auto obj = el->InsertNewChildElement("Entity");


		obj->SetAttribute("Type", "GameObject");


		obj->SetAttribute("Name", it->Name().c_str());

		const auto childEl = obj->InsertNewChildElement("Geometry");

		if (it->Material()->HasNormals())
		{
			std::string id = it->Mesh()->MeshFileName();

			childEl->SetAttribute("ID", id.c_str());
		}
		else
		{
			childEl->SetAttribute("Mesh", it->Mesh()->MeshFileName().c_str());
			childEl->SetAttribute("Diffuse", it->TextureFileName().c_str());
		}

		SavePositionRotationScale(obj, it);
		
	}

	//----------------------------------------------------
	//	Camera
	//----------------------------------------------------

	const auto camera = ptrScene->mCamera.get();

	const auto obj = el->InsertNewChildElement("Entity");
	obj->SetAttribute("Type", "Camera");

	//save position, position
	auto childEl = obj->InsertNewChildElement("Position");
	SaveVector3(camera->Position(), childEl);
	childEl = obj->InsertNewChildElement("Rotation");
	SaveVector3(ToDegrees(camera->Rotation()), childEl);
}

void CDX12Importer::SaveVector3(CVector3 v, tinyxml2::XMLElement* el)
{
	el->SetAttribute("X", v.x);
	el->SetAttribute("Y", v.y);
	el->SetAttribute("Z", v.z);
}

bool CDX12Importer::ParseScene(tinyxml2::XMLElement* sceneEl, CDX12Scene* scene)
{
	auto element = sceneEl->FirstChildElement();

	while (element != nullptr)
	{
		std::string elementName = element->Name();

		if (elementName == "Entities")
		{
			try
			{
				ParseEntities(element, scene);
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(e.what());
			}
		}
		else if (elementName == "PostProcessingEffects")
		{
			ParsePostProcessingEffects(element, scene);
		}
		element = element->NextSiblingElement();
	}

	return true;
}

void CDX12Importer::LoadObject(tinyxml2::XMLElement* currEntity, CDX12Scene* scene)
{
	std::string ID;
	std::string mesh;
	std::string name;
	std::string diffuse;

	CVector3 pos = { 0,0,0 };
	CVector3 rot = { 0,0,0 };
	auto scale = 1.0f;

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

	const auto ambientMapEl = currEntity->FirstChildElement("AmbientMap");

	bool enabled = false;
	int size = 1;

	if (ambientMapEl)
	{
		enabled = ambientMapEl->FindAttribute("Enabled")->BoolValue();
		size = ambientMapEl->FindAttribute("Size")->IntValue();
	}

	// Create objects
	CDX12GameObject* obj;

	try
	{
		if (ID.empty())
		{
			obj = new CDX12GameObject(mEngine, mesh, name, diffuse, pos, rot, scale);

		}
		else
		{
			obj = new CDX12GameObject(mEngine, ID, name, pos, rot, scale);
		}
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(std::string(e.what()) + " of object " + name);
	}
	
	// Add it to the object manager
	scene->mObjManager->AddObject(obj);
}

void CDX12Importer::LoadLight(tinyxml2::XMLElement* currEntity, CDX12Scene* scene)
{
	std::string mesh;
	std::string name;
	std::string diffuse;

	CVector3 colour = { 0,0,0 };
	float strength = 0;
	CVector3 pos = { 0,0,0 };
	CVector3 rot = { 0,0,0 };
	auto scale = 1.0f;

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
	try
	{
		const auto obj = new CDX12Light(mEngine, mesh, name, diffuse, colour, strength, pos, rot, scale);

		scene->mObjManager->AddLight(obj);
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(std::string(e.what()) + " of object " + name);
	}
}

void CDX12Importer::LoadCamera(tinyxml2::XMLElement* currEntity, CDX12Scene* scene)
{
	std::string mesh;
	std::string diffuse;
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

	scene->mCamera = std::make_unique<CCamera>(pos, rot, FOV, aspectRatio, nearClip, farClip);

	if (!scene->mCamera)
	{
		throw std::runtime_error("Error initializing camera");
	}
}

bool CDX12Importer::ParseEntities(tinyxml2::XMLElement* entitiesEl, CDX12Scene* scene)
{
	auto currEntity = entitiesEl->FirstChildElement();

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
					LoadObject(currEntity, scene);
				}
				else if (typeValue == "Light")
				{
					LoadLight(currEntity, scene);
				}
				else if (typeValue == "PointLight")
				{
					//LoadPointLight(currEntity, scene);
				}
				else if (typeValue == "DirectionalLight")
				{
					//LoadDirLight(currEntity, scene);
				}
				else if (typeValue == "SpotLight")
				{
					//LoadSpotLight(currEntity, scene);
				}
				else if (typeValue == "Sky")
				{
					//LoadSky(currEntity, scene);
				}
				else if (typeValue == "Plant")
				{
					//LoadPlant(currEntity, scene);
				}
				else if (typeValue == "Camera")
				{
					LoadCamera(currEntity, scene);
				}
			}
		}
		currEntity = currEntity->NextSiblingElement();
	}
	return true;
}
