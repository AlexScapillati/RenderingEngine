#pragma once
#include "DX12Engine.h"
#include "DX12Texture.h"

class CDX12Material
{

public:

	//-------------------------------------
	// Construction / Usage
	//-------------------------------------

	// Main Constructor
	// Requires a vector of filemaps
	// Formats: NAME_RESOLUTION_TYPE.EXTENTION
	// Types supported: Albedo,AmbientOccusion,Displacement,Roughness,Metallness
	// It will set automatically the correct shaders depending on the use (Normals = PBR / No Normals = PBRNoNormals)
	CDX12Material(std::vector<std::string> fileMaps, CDX12Engine* engine);

	// Copy Constuctor
	// Deep Copy
	CDX12Material(CDX12Material& m);

	~CDX12Material() = default;

	// Return if the material has a normal map
	bool HasNormals() const { return mHasNormals; }

	// Set the shaders
	// Set the maps to the shader
	// Optionally decide to set depth only shaders
	void RenderMaterial() const;

	auto& TextureFileNames() { return mMapsStr; }

private:

	CDX12Engine* mEngine;

	std::vector<std::string> mMapsStr;

	bool mHasNormals;

	std::unique_ptr<CDX12Texture> mAlbedo, mDisplacement, mRoughness, mAo, mNormal, mMetalness;

	void LoadMaps(std::vector<std::string>& maps);
};


