//--------------------------------------------------------------------------------------
// Class encapsulating a model
//--------------------------------------------------------------------------------------
// Holds a pointer to a mesh as well as position, rotation and scaling, which are converted to a world matrix when required
// This is more of a convenience class, the Mesh class does most of the difficult work.

#pragma once

#include <memory>

#include "../../Common/CGameObject.h"
#include "../../Math/CVector3.h"

#include "../DX12Mesh.h"

enum KeyCode;

namespace DX12
{
class CDX12Material;
class CDX12Engine;

	class CDX12GameObject : virtual public CGameObject
	{
	public:
		~CDX12GameObject() override = default;

		//-------------------------------------
		// Construction / Usage
		//-------------------------------------

		// Copy constructor
		// Deep copy
		CDX12GameObject(CDX12GameObject&);

		// Simple object constructor
		// A mesh and a diffuse map are compulsory to render a model
		CDX12GameObject(CDX12Engine* engine,
			const std::string& mesh,
			const std::string& name,
			const std::string& diffuseMap,
			CVector3           position = { 0,0,0 },
			CVector3           rotation = { 0,0,0 },
			float              scale = 1);


		// "Smart" Constructor
		// Given an ID (That could be a file or a directory) this constructor will import all the files in that folder or the files that will contain that id
		// A GameObject to be rendered needs at least a mesh and a texture
		// Formats:
		// Folders: NAME_ID_TYPE
		// Meshes:	NAME_LOD_VARIATION.EXTENTION
		// Textures: ID_RESOLUTION_TYPE.EXTENTION
		CDX12GameObject(CDX12Engine* engine,
			std::string        id,
			const std::string& name,
			CVector3           position = { 0,0,0 },
			CVector3           rotation = { 0,0,0 },
			float              scale = 1);


		CDX12Material* Material() const;

		// Delete the current mesh and load the given one. It will not delete the current if the filename is wrong
		void LoadNewMesh(std::string newMesh) override;

		// Render the object
		void Render(bool basicGeometry = false) override;

		//-------------------------------------
		// Private data / members
		//-------------------------------------

	protected:

		CDX12Engine* mEngine;

		// The acutual mesh class
		std::unique_ptr<CDX12Mesh> mMesh;
	};

	class CDX12Plant : public CDX12GameObject, public CPlant
	{
		public:
			explicit CDX12Plant(CDX12GameObject& cdx12GameObject)
				: CDX12GameObject(cdx12GameObject)
			{
			}

			CDX12Plant(CDX12Engine* engine, const std::string& mesh, const std::string& name, const std::string& diffuseMap, const CVector3& position, const CVector3& rotation, float scale)
				: CDX12GameObject(engine, mesh, name, diffuseMap, position, rotation, scale)
			{
			}

			CDX12Plant(CDX12Engine* engine, const std::string& id, const std::string& name, const CVector3& position, const CVector3& rotation, float scale)
				: CDX12GameObject(engine, id, name, position, rotation, scale)
			{
			}

			void LoadNewMesh(std::string newMesh) override;
			void                                          Render(bool basicGeometry) override;
	};
}
