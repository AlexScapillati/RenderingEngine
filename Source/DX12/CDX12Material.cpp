#include "CDX12Material.h"

#include "DX12DescriptorHeap.h"
#include "DX12Mesh.h"
#include "DX12Texture.h"


CDX12Material::CDX12Material(std::vector<std::string> fileMaps, CDX12Engine* engine)
{
	mEngine = engine;

	mHasNormals = false;

	mMapsStr = fileMaps;

	//load all the textures
	try
	{
		LoadMaps(mMapsStr);
	}
	catch (const std::runtime_error& e)
	{
		throw std::runtime_error(e.what());
	}
}

CDX12Material::CDX12Material(CDX12Material& m)
{
	mEngine = m.mEngine;

	mHasNormals = false;

	mMapsStr = m.mMapsStr;

	try
	{
		LoadMaps(mMapsStr);
	}
	catch (const std::runtime_error& e)
	{
		throw std::runtime_error(e.what());
	}
}

CDX12Material::~CDX12Material()
{
}

void CDX12Material::RenderMaterial() const
{
	// Set textures to the pixel shader
	{
		mEngine->mSRVDescriptorHeap->Set();

		if (mAlbedo)		mAlbedo->Set(3);
		if (mRoughness)		mRoughness->Set(4);
		if (mAo)			mAo->Set(5);
		if (mDisplacement)	mDisplacement->Set(6);
		if (mHasNormals)	mNormal->Set(7);
		if (mMetalness)		mMetalness->Set(8);
	}
}

std::vector<void*> CDX12Material::GetTextureSRV() const
{
	std::vector<void*> r;

	if (mAlbedo)		r.push_back((void*)mAlbedo->mHandle.mGpu.ptr);
	if (mRoughness)		r.push_back((void*)mRoughness->mHandle.mGpu.ptr);
	if (mAo)			r.push_back((void*)mAo->mHandle.mGpu.ptr);
	if (mDisplacement)	r.push_back((void*)mDisplacement->mHandle.mGpu.ptr);
	if (mNormal)		r.push_back((void*)mNormal->mHandle.mGpu.ptr);
	if (mMetalness)		r.push_back((void*)mMetalness->mHandle.mGpu.ptr);

	return r;
}

void CDX12Material::LoadMaps(std::vector<std::string>& fileMaps)
{
	// If the vector is empty
	if (fileMaps.empty())
	{
		// Throw an exception
		throw std::runtime_error("No maps found");
	}
	// If the vector is size 1,
	if (fileMaps.size() == 1)
	{
		//assume it is an diffuse specular map
		mAlbedo = std::make_unique<CDX12Texture>(mEngine,fileMaps.front());
	}
	else
	{
		//for each file in the vector with the same name as the mesh one
		for (auto fileName : fileMaps)
		{
			auto originalFileName = fileName;

			////load it

			if (fileName.find("Albedo") != std::string::npos)
			{
				mAlbedo = std::make_unique<CDX12Texture>(mEngine, originalFileName);
			}
			else if (fileName.find("Roughness") != std::string::npos)
			{
				//roughness map
				mRoughness = std::make_unique<CDX12Texture>(mEngine, originalFileName);
			}
			else if (fileName.find("AO") != std::string::npos)
			{
				//ambient occlusion map
				mAo = std::make_unique<CDX12Texture>(mEngine, originalFileName);
			}
			else if (fileName.find("Displacement") != std::string::npos)
			{
				//found displacement map
				mDisplacement = std::make_unique<CDX12Texture>(mEngine, originalFileName);
			}
			else if (fileName.find("Normal") != std::string::npos)
			{
				//normal map
				mNormal = std::make_unique<CDX12Texture>(mEngine, originalFileName);

				mHasNormals = true;
			}
			else if (fileName.find("Metalness") != std::string::npos)
			{
				// Metalness Map
				mMetalness = std::make_unique<CDX12Texture>(mEngine, originalFileName);
			}
		}
	}
}
