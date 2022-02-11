#pragma once

#include "CDX12Common.h"

#include "DX12Engine.h"

#include "ResourceUploadBatch.h"
#include "../External/DirectXTK12/Inc/DirectXHelpers.h"
#include "../External/DirectXTK12/Inc/DDSTextureLoader.h"
#include "../External/DirectXTK12/Inc/WICTextureLoader.h"

class CDX12Texture
{
public:

	CDX12Engine* mPtrEngine = nullptr;
	Resource mResource;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mCpuSRVDescriptorHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mGpuSRVDescriptorHandle;
	INT mDescriptorIndex;

	D3D12_RESOURCE_STATES mCurrentResourceState;

	CDX12Texture(CDX12Engine* engine, std::string& filename)
	{
		mPtrEngine = engine;

		LoadTexture(filename);
	}

	CDX12Texture(CDX12Engine* engine, D3D12_RESOURCE_DESC desc)
	{
		mPtrEngine = engine;

		CreateTexture(desc);
	}
	

protected:

	void LoadTexture(std::string& filename)
	{
		filename = mPtrEngine->GetMediaFolder() + filename;

		DirectX::ResourceUploadBatch resourceUpload(mPtrEngine->mDevice.Get());

		resourceUpload.Begin();

		ComPtr<ID3D12Resource> textureResource;

		const auto resourceFlags = D3D12_RESOURCE_FLAG_NONE;

		const bool isSrgb = filename.find("Albedo") != std::string::npos;

		auto ddsFlags = isSrgb ? DirectX::DDS_LOADER_FORCE_SRGB : DirectX::DDS_LOADER_DEFAULT;
		auto wicFlags = isSrgb ? DirectX::WIC_LOADER_FORCE_SRGB : DirectX::WIC_LOADER_DEFAULT;

		ddsFlags |= DirectX::DDS_LOADER_MIP_AUTOGEN;
		wicFlags |= DirectX::WIC_LOADER_MIP_AUTOGEN;

		// DDS files need a different function from other files
		std::string dds = ".dds"; // So check the filename extension (case insensitive)
		if (filename.size() >= 4 &&
			std::equal(dds.rbegin(), dds.rend(), filename.rbegin(), [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); }))
		{
			if (FAILED(DirectX::CreateDDSTextureFromFileEx(
				mPtrEngine->mDevice.Get(),
				resourceUpload,
				ATL::CA2W(filename.c_str()),
				0,
				resourceFlags,
				ddsFlags,
				textureResource.GetAddressOf())))
			{
				throw std::runtime_error("Failed to load image: " + filename);
			}
		}
		else
		{
			if (FAILED(DirectX::CreateWICTextureFromFileEx(
				mPtrEngine->mDevice.Get(),
				resourceUpload,
				ATL::CA2W(filename.c_str()),
				0,
				resourceFlags,
				wicFlags,
				textureResource.GetAddressOf())))
			{
				throw std::runtime_error("Failed to load image: " + filename);
			}
		}

		mCpuSRVDescriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mPtrEngine->mSRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			mPtrEngine->mNumTextures, mPtrEngine->mSRVSize);

		mDescriptorIndex = mPtrEngine->mNumTextures;

		mGpuSRVDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mPtrEngine->mSRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), mDescriptorIndex, mPtrEngine->mSRVSize);

		DirectX::CreateShaderResourceView(mPtrEngine->mDevice.Get(), textureResource.Get(), mCpuSRVDescriptorHandle);

		const auto uploadResourceFinished = resourceUpload.End(mPtrEngine->mCommandQueue.Get());

		uploadResourceFinished.wait();

		mPtrEngine->mNumTextures++;

		mResource = textureResource;
	}

	void CreateTexture(D3D12_RESOURCE_DESC desc)
	{
		D3D12_CLEAR_VALUE clearValue = { DXGI_FORMAT_R8G8B8A8_UNORM, { 0.f, 0.f, 0.f, 0.f } };

		auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		mPtrEngine->mDevice->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&desc,
			D3D12_RESOURCE_STATE_COMMON,
			&clearValue,
			IID_PPV_ARGS(mResource.GetAddressOf()));

		mCpuSRVDescriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mPtrEngine->mSRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			mPtrEngine->mNumTextures, mPtrEngine->mSRVSize);

		mDescriptorIndex = mPtrEngine->mNumTextures;

		mGpuSRVDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mPtrEngine->mSRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), mDescriptorIndex, mPtrEngine->mSRVSize);

		mPtrEngine->mNumTextures++;

		DirectX::CreateShaderResourceView(mPtrEngine->mDevice.Get(), mResource.Get(), mCpuSRVDescriptorHandle);
	}




};

class CDX12RenderTarget : public CDX12Texture
{
public:

	CDX12RenderTarget(CDX12Engine* engine, D3D12_RESOURCE_DESC desc) : CDX12Texture(engine, desc)
	{
		CreateRenderTargetView();
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE mCpuRTVDescriptorHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mGpuRTVDescriptorHandle;
	INT mRTVDescriptorIndex;

private:


	void CreateRenderTargetView()
	{
		mCpuRTVDescriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mPtrEngine->mRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			mPtrEngine->mNumTextures, mPtrEngine->mSRVSize);

		mDescriptorIndex = mPtrEngine->mRTVTop;

		mGpuRTVDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mPtrEngine->mRTVDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), mDescriptorIndex, mPtrEngine->mRTVSize);

		mPtrEngine->mRTVTop++;

		mPtrEngine->mDevice->CreateRenderTargetView(mResource.Get(), nullptr, mCpuRTVDescriptorHandle);
	}
};
