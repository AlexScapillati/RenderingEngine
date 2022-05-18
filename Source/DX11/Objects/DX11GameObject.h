//--------------------------------------------------------------------------------------
// Class encapsulating a model
//--------------------------------------------------------------------------------------
// Holds a pointer to a mesh as well as position, rotation and scaling, which are converted to a world matrix when required
// This is more of a convenience class, the Mesh class does most of the difficult work.

#pragma once


#include <d3d11.h>
#include <memory>
#include <wrl/client.h>

#include "../DX11Material.h"
#include "../Mesh.h"
#include "..\..\Common/CGameObject.h"


namespace DX11
{
	class CDX11Engine;

	class CDX11GameObject : virtual public CGameObject
	{
		public:

			//-------------------------------------
			// Construction / Usage
			//-------------------------------------

			// Copy constructor
			// Deep copy
			CDX11GameObject(CDX11GameObject&);

			// Simple object constructor
			// A mesh and a diffuse map are compulsory to render a model
			CDX11GameObject(CDX11Engine* engine, const std::string& mesh, const std::string& name, const std::string& diffuseMap, CVector3 position = { 0,0,0 }, CVector3 rotation = { 0,0,0 }, float scale = 1);


			// "Smart" Constructor
			// Given an ID (That could be a file or a directory) this constructor will import all the files in that folder or the files that will contain that id
			// A GameObject to be rendered needs at least a mesh and a texture
			// Formats:
			// Folders: NAME_ID_TYPE
			// Meshes:	NAME_LOD_VARIATION.EXTENTION
			// Textures: ID_RESOLUTION_TYPE.EXTENTION
			CDX11GameObject(CDX11Engine* engine,const std::string& id, std::string name, CVector3 position = { 0,0,0 }, CVector3 rotation = { 0,0,0 }, float scale = 1);

			CDX11Mesh* Mesh() const;
			CDX11Material* Material() const;

			void Render(bool basicGeometry = false) override;

			//-------------------------------------
			// Ambient Map (Global Illumination)
			//-------------------------------------
			// The Ambient Map
			// Its purpose is to render the scene surrounding the object to a cubemap
			// Then it will send the cubemap to the shader and display reflexes on the object
			struct sAmbientMap
			{
				CDX11Engine* mEngine;

				// Initialize the textures
				void Init(CDX11Engine* engine);

				bool                                             enabled;
				ComPtr<ID3D11Texture2D>          map;              // The actual texture stored on the GPU side (cubemap)
				ComPtr<ID3D11ShaderResourceView> mapSRV;           // The texture in the shader resource view format, to send it to the shader
				ComPtr<ID3D11Texture2D>          depthStencilMap;  // The depth stencil texture 
				ComPtr<ID3D11DepthStencilView>   depthStencilView; // The depth stencil view to set it as the render target
				ComPtr<ID3D11RenderTargetView>   RTV[6];           // The 6 different render targets, one for each face, to bind to the render target
				UINT                                             size;             // Size of each face of the cubemap

				// Getters and Setters for the size
				UINT Size() const;
				void SetSize(UINT s);
			}        mAmbientMap;

			// Returns if the ambient map is enabled
			bool& AmbientMapEnabled();

			// Returns the actual ambient map struct
			sAmbientMap* AmbientMap();

			// Returns the ambient map shader resource view (for passing it to the shaders)
			ID3D11ShaderResourceView* AmbientMapSRV() const;

			void LoadNewMesh(std::string newMesh) final;

			ID3D11ShaderResourceView* TextureSRV() const;

			// Render the whole scene from the model perspective. Very expensive, needs optimization
			void RenderToAmbientMap() override;

		protected:
			//-------------------------------------
			// Private data / members
			//-------------------------------------

			CDX11Engine* mEngine;

			// The actual mesh
			std::unique_ptr<CDX11Mesh> mMesh;

			// The material
			// It will hold all the textures and send them to the shader with RenderMaterial()
			std::unique_ptr<CDX11Material> mMaterial;
	};
}
