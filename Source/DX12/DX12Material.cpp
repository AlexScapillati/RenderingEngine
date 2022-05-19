
#include "DX12Texture.h"
#include "DX12DescriptorHeap.h"
#include "CDX12Material.h"
#include "DX12Engine.h"
#include "DX12ConstantBuffer.h"

namespace DX12
{
	CDX12Material::CDX12Material(std::vector<std::string>& fileMaps, CDX12Engine* engine)
	{
		mEngine = engine;

		mMapsDescriptorHeap = mEngine->mSRVDescriptorHeap.get();

		mHasNormals = false;

		mMapsStr = fileMaps;

		//load all the textures
		try
		{
			LoadMaps(fileMaps);
		}
		catch (const std::runtime_error& e)
		{
			throw std::runtime_error(e.what());
		}


		std::pair<int, int> dims[6];

		mMaterialCB = std::make_unique<CDX12ConstantBuffer>(mEngine, mMapsDescriptorHeap, sizeof(int) * 12);

		if (mAlbedo)		dims[0] = { mAlbedo->mDesc.Width, mAlbedo->mDesc.Height };
		if (mNormal)		dims[1] = { mNormal->mDesc.Width, mNormal->mDesc.Height };
		if (mDisplacement)	dims[2] = { mDisplacement->mDesc.Width, mDisplacement->mDesc.Height };
		if (mRoughness)		dims[3] = { mRoughness->mDesc.Width, mRoughness->mDesc.Height };
		if (mMetalness)		dims[4] = { mMetalness->mDesc.Width, mMetalness->mDesc.Height };
		if (mAo)			dims[5] = { mAo->mDesc.Width, mAo->mDesc.Height };

		mMaterialCB->Copy(dims);
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
		mMapsDescriptorHeap = nullptr;
	}

	void CDX12Material::RenderMaterial() const
	{
		// Set textures to the pixel shader
		{
			// different pso will have different root parameter index
			
			if (mAlbedo)		mAlbedo->Set( 6);
			if (mRoughness)		mRoughness->Set( 7);
			if (mAo)			mAo->Set( 8);
			if (mDisplacement)	mDisplacement->Set( 9);
			if (mHasNormals)	mNormal->Set( 10);
			if (mMetalness)		mMetalness->Set(11);
		}
	}

	std::vector<void*> CDX12Material::GetTextureSRV() const
	{
		std::vector<void*> r;

		if (mAlbedo)		r.push_back((void*)(mAlbedo->mSrvHeap->Get(mAlbedo->mSrvHandle).mGpu.ptr));
		if (mRoughness)		r.push_back((void*)(mRoughness->mSrvHeap->Get(mRoughness->mSrvHandle).mGpu.ptr));
		if (mAo)			r.push_back((void*)(mAo->mSrvHeap->Get(mAo->mSrvHandle).mGpu.ptr));
		if (mDisplacement)	r.push_back((void*)(mDisplacement->mSrvHeap->Get(mDisplacement->mSrvHandle).mGpu.ptr));
		if (mNormal)		r.push_back((void*)(mNormal->mSrvHeap->Get(mNormal->mSrvHandle).mGpu.ptr));
		if (mMetalness)		r.push_back((void*)(mMetalness->mSrvHeap->Get(mMetalness->mSrvHandle).mGpu.ptr));
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
			mAlbedo = std::make_unique<CDX12Texture>(mEngine, fileMaps.front(),mMapsDescriptorHeap);
		}
		else
		{
			//for each file in the vector with the same name as the mesh one
			for (std::string& fileName : fileMaps)
			{
				////load it

				if (fileName.find("Albedo") != std::string::npos)
				{
					mAlbedo = std::make_unique<CDX12Texture>(mEngine, fileName, mMapsDescriptorHeap);
				}
				else if (fileName.find("Roughness") != std::string::npos)
				{
					//roughness map
					mRoughness = std::make_unique<CDX12Texture>(mEngine, fileName, mMapsDescriptorHeap);
				}
				else if (fileName.find("AO") != std::string::npos)
				{
					//ambient occlusion map
					mAo = std::make_unique<CDX12Texture>(mEngine, fileName, mMapsDescriptorHeap);
				}
				else if (fileName.find("Displacement") != std::string::npos)
				{
					//found displacement map
					mDisplacement = std::make_unique<CDX12Texture>(mEngine, fileName, mMapsDescriptorHeap);
				}
				else if (fileName.find("Normal") != std::string::npos)
				{
					//normal map
					mNormal = std::make_unique<CDX12Texture>(mEngine, fileName, mMapsDescriptorHeap);

					mHasNormals = true;
				}
				else if (fileName.find("Metalness") != std::string::npos)
				{
					// Metalness Map
					mMetalness = std::make_unique<CDX12Texture>(mEngine, fileName, mMapsDescriptorHeap);
				}
			}
		}
	}
}
