#include "CDX12Material.h"

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

void CDX12Material::RenderMaterial() const
{
	// Set texture to the pixel shader
	{
		const auto prevState = mEngine->mCurrentResourceState;
		mEngine->Barrier(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		
		ID3D12DescriptorHeap* ppHeaps[] = { mEngine->mSRVDescriptorHeap.Get() };
		mEngine->mCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		if(mAlbedo)			mEngine->mCommandList->SetGraphicsRootDescriptorTable(2, mAlbedo->mGpuSRVDescriptorHandle);
		if(mRoughness)		mEngine->mCommandList->SetGraphicsRootDescriptorTable(3, mRoughness->mGpuSRVDescriptorHandle);
		if(mAo)				mEngine->mCommandList->SetGraphicsRootDescriptorTable(4, mAo->mGpuSRVDescriptorHandle);
		if(mDisplacement)	mEngine->mCommandList->SetGraphicsRootDescriptorTable(5, mDisplacement->mGpuSRVDescriptorHandle);
		if(mHasNormals)		mEngine->mCommandList->SetGraphicsRootDescriptorTable(6, mNormal->mGpuSRVDescriptorHandle);
		if(mMetalness)		mEngine->mCommandList->SetGraphicsRootDescriptorTable(7, mMetalness->mGpuSRVDescriptorHandle);

		mEngine->Barrier(prevState);
	}
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
