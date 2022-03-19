

#include "LevelImporter.h"

#include <stdexcept>

#include "CGameObjectManager.h"
#include "../Engine.h"
#include "..\Common/Camera.h"
#include "..\Common/CScene.h"

#include "../Common/CLight.h"
#include "../Common/CPostProcess.h"
#include "../DX11/DX11Engine.h"

bool CLevelImporter::LoadScene(const std::string& level )
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

void CLevelImporter::SaveScene(std::string& fileName /* ="" */)
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

void CLevelImporter::SavePositionRotationScale(tinyxml2::XMLElement* obj,
	CGameObject* it)
{
	//save position, position and scale
	auto childEl = obj->InsertNewChildElement("Position");
	SaveVector3(it->Position(), childEl);
	childEl = obj->InsertNewChildElement("Rotation");
	SaveVector3(ToDegrees(it->Rotation()), childEl);
	childEl = obj->InsertNewChildElement("Scale");
	SaveVector3(it->Scale(), childEl);
}

void CLevelImporter::SaveObjects(tinyxml2::XMLElement* el)
{
	//----------------------------------------------------
	//	Game Objects
	//----------------------------------------------------

	for (const auto it : ptrScene->GetObjectManager()->mObjects)
	{
		const auto obj = el->InsertNewChildElement("Entity");

		if (auto sky = dynamic_cast<CSky*>(it))
		{
			obj->SetAttribute("Type", "Sky");
		}
		else if (auto plant = dynamic_cast<CPlant*>(it))
		{
			obj->SetAttribute("Type", "Plant");
		}
		else
		{
			obj->SetAttribute("Type", "GameObject");
		}

		obj->SetAttribute("Name", it->Name().c_str());

		auto childEl = obj->InsertNewChildElement("Geometry");

		//if (dynamic_cast<DX11::CDX11GameObject*>(it)->Material()->HasNormals())
		//{
		//	std::string id = dynamic_cast<DX11::CDX11GameObject*>(it)->Mesh()->MeshFileName();

		//	childEl->SetAttribute("ID", id.c_str());
		//}
		//else
		//{
		//	childEl->SetAttribute("Mesh", dynamic_cast<DX11::CDX11GameObject*>(it)->Mesh()->MeshFileName().c_str());
		//	childEl->SetAttribute("Diffuse", dynamic_cast<DX11::CDX11GameObject*>(it)->TextureFileName().c_str());
		//}

		SavePositionRotationScale(obj, it);

		//// Ambient Map
		//if (dynamic_cast<DX11::CDX11GameObject*>(it)->AmbientMapEnabled())
		//{
		//	childEl = obj->InsertNewChildElement("AmbientMap");
		//	childEl->SetAttribute("Enabled", dynamic_cast<DX11::CDX11GameObject*>(it)->AmbientMapEnabled());
		//	childEl->SetAttribute("Size", (int)dynamic_cast<DX11::CDX11GameObject*>(it)->AmbientMap()->Size());
		//}
	}

	//----------------------------------------------------
	//	Simple Lights
	//----------------------------------------------------

	for (const auto it : ptrScene->GetObjectManager()->mLights)
	{
		const auto obj = el->InsertNewChildElement("Entity");
		obj->SetAttribute("Type", "Light");
		obj->SetAttribute("Name", dynamic_cast<CGameObject*>(it)->Name().c_str());

		auto childEl = obj->InsertNewChildElement("Geometry");

		childEl->SetAttribute("Mesh", dynamic_cast<CGameObject*>(it)->MeshFileNames().c_str());
		childEl->SetAttribute("Diffuse", dynamic_cast<CGameObject*>(it)->TextureFileName().c_str());

		//save position, position and scale
		SavePositionRotationScale(obj, dynamic_cast<CGameObject*>(it));

		//save colour and strength
		childEl = obj->InsertNewChildElement("Colour");
		SaveVector3(it->GetColour(), childEl);
		childEl = obj->InsertNewChildElement("Strength");
		childEl->SetAttribute("S", it->GetStrength());
	}

	//----------------------------------------------------
	//	Spot Lights
	//----------------------------------------------------

	for (const auto it : ptrScene->GetObjectManager()->mSpotLights)
	{
		const auto obj = el->InsertNewChildElement("Entity");
		obj->SetAttribute("Type", "SpotLight");
		obj->SetAttribute("Name", dynamic_cast<CGameObject*>(it)->Name().c_str());

		auto childEl = obj->InsertNewChildElement("Geometry");

		childEl->SetAttribute("Mesh", dynamic_cast<CGameObject*>(it)->MeshFileNames().c_str());
		childEl->SetAttribute("Diffuse", dynamic_cast<CGameObject*>(it)->TextureFileName().c_str());

		//save position, position and scale
		SavePositionRotationScale(obj, dynamic_cast<CGameObject*>(it));

		//save colour and strength
		childEl = obj->InsertNewChildElement("Colour");
		SaveVector3(it->GetColour(), childEl);
		childEl = obj->InsertNewChildElement("Strength");
		childEl->SetAttribute("S", it->GetStrength());
	}

	//----------------------------------------------------
	//	Directional Lights
	//----------------------------------------------------

	for (const auto it : ptrScene->GetObjectManager()->mDirLights)
	{
		const auto obj = el->InsertNewChildElement("Entity");
		obj->SetAttribute("Type", "DirectionalLight");
		obj->SetAttribute("Name", dynamic_cast<CGameObject*>(it)->Name().c_str());

		auto childEl = obj->InsertNewChildElement("Geometry");

		childEl->SetAttribute("Mesh", dynamic_cast<CGameObject*>(it)->MeshFileNames().c_str());
		childEl->SetAttribute("Diffuse", dynamic_cast<CGameObject*>(it)->TextureFileName().c_str());

		//save rotation and scale
		SavePositionRotationScale(obj, dynamic_cast<CGameObject*>(it));

		//save colour and strength
		childEl = obj->InsertNewChildElement("Colour");
		SaveVector3(it->GetColour(), childEl);
		childEl = obj->InsertNewChildElement("Strength");
		childEl->SetAttribute("S", it->GetStrength());
	}

	//----------------------------------------------------
	//	Point Lights
	//----------------------------------------------------

	for (const auto it : ptrScene->GetObjectManager()->mPointLights)
	{
		const auto obj = el->InsertNewChildElement("Entity");
		obj->SetAttribute("Type", "PointLight");
		obj->SetAttribute("Name", dynamic_cast<CGameObject*>(it)->Name().c_str());

		auto childEl = obj->InsertNewChildElement("Geometry");

		childEl->SetAttribute("Mesh", dynamic_cast<CGameObject*>(it)->MeshFileNames().c_str());
		childEl->SetAttribute("Diffuse", dynamic_cast<CGameObject*>(it)->TextureFileName().c_str());

		//save position, position and scale
		SavePositionRotationScale(obj, dynamic_cast<CGameObject*>(it));

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

void CLevelImporter::SaveVector3(CVector3              v,
	tinyxml2::XMLElement* el)
{
	el->SetAttribute("X", v.x);
	el->SetAttribute("Y", v.y);
	el->SetAttribute("Z", v.z);
}

bool CLevelImporter::ParseScene(tinyxml2::XMLElement* sceneEl)
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

void CLevelImporter::LoadObject(tinyxml2::XMLElement* currEntity)
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
	int  size = 1;

	if (ambientMapEl)
	{
		enabled = ambientMapEl->FindAttribute("Enabled")->BoolValue();
		size = ambientMapEl->FindAttribute("Size")->IntValue();
	}

	// Create objects
	CGameObject* obj = nullptr;

	if (ID.empty())
	{
		obj = mEngine->CreateObject(mesh, name, diffuse, pos, rot, scale);

		// Add it to the object manager
		if (obj) scene->GetObjectManager()->AddObject(obj);
	}
	else
	{
		obj = mEngine->CreateObject(ID, name, pos, rot, scale);

		// Add it to the object manager
		if (obj) scene->GetObjectManager()->AddObject(obj);
	}


	// Set ambient map values
	//obj->AmbientMapEnabled() = enabled;
	//obj->AmbientMap()->SetSize(size);

}

void CLevelImporter::LoadPointLight(tinyxml2::XMLElement* currEntity)
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

	scene->GetObjectManager()->AddPointLight(obj);
}

void CLevelImporter::LoadLight(tinyxml2::XMLElement* currEntity)
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

	scene->GetObjectManager()->AddLight(obj);
}


void CLevelImporter::LoadSpotLight(tinyxml2::XMLElement* currEntity)
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

	scene->GetObjectManager()->AddSpotLight(obj);
}


void CLevelImporter::LoadDirLight(tinyxml2::XMLElement* currEntity)
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

	scene->GetObjectManager()->AddDirLight(obj);
}


void CLevelImporter::LoadSky(tinyxml2::XMLElement* currEntity)
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

	CSky* obj = mEngine->CreateSky(mesh, name, diffuse, pos, rot, scale);

	scene->GetObjectManager()->AddSky(obj);

}

void CLevelImporter::LoadCamera(tinyxml2::XMLElement* currEntity)
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

	mEngine->GetScene()->SetCamera(new CCamera(pos, rot, FOV, aspectRatio, nearClip, farClip));

	if (!mEngine->GetScene()->GetCamera())
	{
		throw std::runtime_error("Error initializing camera");
	}
}

void CLevelImporter::LoadPlant(tinyxml2::XMLElement* currEntity)
{
	std::string ID;
	std::string name;

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
		const auto obj = mEngine->CreatePlant(ID, name, pos, rot, scale);
		scene->GetObjectManager()->AddPlant(obj);
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(std::string(e.what()) + " of object " + name);
	}
}

void CLevelImporter::SavePostProcessingEffect(tinyxml2::XMLElement* curr)
{


	// Save the Type and mode for every effect
	for (const auto& pp : mPostProcessingFilters)
	{
		const auto ppEl = curr->InsertNewChildElement("Effect");

		// Cast the type string with the corresponding enum
		ppEl->SetAttribute("Type", mPostProcessStrings[(int)pp.type].c_str());
		ppEl->SetAttribute("Mode", mPostProcessModeStrings[(int)pp.mode].c_str());
	}

	// Save settings
	// Create an array of floats
	float settings[sizeof(DX11::PostProcessingConstants) / sizeof(float)];

	// Copy the postprocessing constants struct in the array of floats with memcpy
	memcpy(settings, &DX11::gPostProcessingConstants, sizeof(DX11::PostProcessingConstants));

	// Insert the settings element
	const auto settingsEl = curr->InsertNewChildElement("Settings");

	// For every setting 
	for (unsigned long long int i = 0; i < ARRAYSIZE(settings); ++i)
	{
		// create a different name for each setting
		std::string name = "setting";
		name.append(std::to_string(i));

		// set the attribute 
		settingsEl->SetAttribute(name.c_str(), settings[i]);
	}
}

void CLevelImporter::ParsePostProcessingEffects(tinyxml2::XMLElement* curr)
{

	auto currEffect = curr->FirstChildElement();

	while (currEffect)
	{
		std::string item = currEffect->Name();

		if (item == "Effect")
		{
			std::string typeValue;
			std::string modeValue;

			const auto type = currEffect->FindAttribute("Type");
			if (type)
			{
				typeValue = type->Value();
			}

			const auto mode = currEffect->FindAttribute("Mode");
			if (mode)
				modeValue = mode->Value();

			PostProcessFilter filter;


			for (unsigned long long int i = 0; i < ARRAYSIZE(mPostProcessModeStrings); ++i)
			{
				if (modeValue._Equal(mPostProcessModeStrings[i]))
				{
					filter.mode = (PostProcessMode)i;
				}
			}

			for (int i = 0; i < ARRAYSIZE(mPostProcessStrings); ++i)
			{
				if (typeValue._Equal(mPostProcessStrings[i]))
				{
					filter.type = (PostProcess)i;
				}
			}

			mPostProcessingFilters.push_back(filter);
		}
		// After Loading all the effects
		// Load the settings
		else if (item == "Settings")
		{
			float values[sizeof(DX11::gPostProcessingConstants) / sizeof(float)];

			for (unsigned long long int i = 0; i < sizeof(DX11::gPostProcessingConstants) / sizeof(float); ++i)
			{
				// get the different name for each setting
				std::string name = "setting";
				name.append(std::to_string(i));

				const auto currSetting = currEffect->FindAttribute(name.c_str());

				values[i] = currSetting->FloatValue();
			}

			memcpy(&DX11::gPostProcessingConstants, values, sizeof(values));
		}
		currEffect = currEffect->NextSiblingElement();
	}
}

bool CLevelImporter::ParseEntities(tinyxml2::XMLElement* entitiesEl)
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
					LoadObject(currEntity );
				}
				else if (typeValue == "Light")
				{
					LoadLight(currEntity );
				}
				else if (typeValue == "PointLight")
				{
					LoadPointLight(currEntity );
				}
				else if (typeValue == "DirectionalLight")
				{
					LoadDirLight(currEntity );
				}
				else if (typeValue == "SpotLight")
				{
					LoadSpotLight(currEntity );
				}
				else if (typeValue == "Sky")
				{
					LoadSky(currEntity );
				}
				else if (typeValue == "Plant")
				{
					LoadPlant(currEntity );
				}
				else if (typeValue == "Camera")
				{
					LoadCamera(currEntity );
				}
			}
		}
		currEntity = currEntity->NextSiblingElement();
	}
	return true;
}
