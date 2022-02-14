
#include "DX12Texture.h"
#include "DX12ConstantBuffer.h"

#include "DirectXHelpers.h"
#include "DDSTextureLoader.h"
#include "DX12DescriptorHeap.h"
#include "WICTextureLoader.h"
#include "ResourceUploadBatch.h"


CDX12Texture::CDX12Texture(CDX12Engine* engine, DX12Common::Resource r)
{
	mPtrEngine = engine;
	r.Swap(mResource);
}

CDX12Texture::CDX12Texture(CDX12Engine* engine, std::string& filename)
{
	mPtrEngine = engine;

	mDescriptorIndex = mPtrEngine->mSRVDescriptorHeap->Top();
	mHandle          = mPtrEngine->mSRVDescriptorHeap->Add();

	LoadTexture(filename);
}

CDX12Texture::CDX12Texture(CDX12Engine* engine, D3D12_RESOURCE_DESC desc)
{
	mPtrEngine = engine;

	mDescriptorIndex = mPtrEngine->mSRVDescriptorHeap->Top();
	mHandle          = mPtrEngine->mSRVDescriptorHeap->Add();

	CreateTexture(desc);
}

CDX12Texture::~CDX12Texture()
{
}

void CDX12Texture::Reset(D3D12_RESOURCE_DESC desc)
{
	mResource->Release();
	CreateTexture(desc);
}

void CDX12Texture::Barrier(D3D12_RESOURCE_STATES after)
{
	if (after == mCurrentResourceState) return;

	const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		mResource.Get(),
		mCurrentResourceState,
		after);

	mPtrEngine->mCommandList->ResourceBarrier(1, &barrier);

	mCurrentResourceState = after;
}

void CDX12Texture::Set(UINT rootParameterIndex)
{
	mPtrEngine->mCommandList->SetGraphicsRootDescriptorTable(rootParameterIndex, mHandle.mGpu);
}

void CDX12Texture::LoadTexture(std::string& filename)
{
	filename = mPtrEngine->GetMediaFolder() + filename;

	DirectX::ResourceUploadBatch resourceUpload(mPtrEngine->mDevice.Get());

	resourceUpload.Begin();

	ComPtr<ID3D12Resource> textureResource;

	const auto resourceFlags = D3D12_RESOURCE_FLAG_NONE;

	const bool isSrgb = false; filename.find("Albedo") != std::string::npos;

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

	DirectX::CreateShaderResourceView(mPtrEngine->mDevice.Get(), textureResource.Get(), mHandle.mCpu);

	const auto uploadResourceFinished = resourceUpload.End(mPtrEngine->mCommandQueue.Get());

	uploadResourceFinished.wait();

	mResource = textureResource;
}

void CDX12Texture::CreateTexture(D3D12_RESOURCE_DESC desc)
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

	mCurrentResourceState = D3D12_RESOURCE_STATE_COMMON;

	DirectX::CreateShaderResourceView(mPtrEngine->mDevice.Get(), mResource.Get(), mHandle.mCpu);
}

CDX12RenderTarget::CDX12RenderTarget(CDX12Engine* engine, Resource r): CDX12Texture(engine,r)
{
	mRTVDescriptorIndex = mPtrEngine->mRTVDescriptorHeap->Top();
	mRTVHandle          = mPtrEngine->mRTVDescriptorHeap->Add();

	mPtrEngine->mDevice->CreateRenderTargetView(mResource.Get(), nullptr, mRTVHandle.mCpu);
}

CDX12RenderTarget::CDX12RenderTarget(CDX12Engine* engine, D3D12_RESOURCE_DESC desc): CDX12Texture(engine, desc)
{
	mRTVDescriptorIndex = mPtrEngine->mRTVDescriptorHeap->Top();

	mRTVHandle = mPtrEngine->mRTVDescriptorHeap->Add();

	mPtrEngine->mDevice->CreateRenderTargetView(mResource.Get(), nullptr, mRTVHandle.mCpu);
}

CDX12RenderTarget::~CDX12RenderTarget()
{
}

void CDX12RenderTarget::Reset(D3D12_RESOURCE_DESC desc) {
	CDX12Texture::Reset(desc);

	mPtrEngine->mDevice->CreateRenderTargetView(mResource.Get(), nullptr, mRTVHandle.mCpu);
}
