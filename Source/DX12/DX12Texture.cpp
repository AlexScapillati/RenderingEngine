#include "DX12Texture.h"

#include "DX12ConstantBuffer.h"
#include "DX12DescriptorHeap.h"
#include "DX12Engine.h"
#include "ResourceUploadBatch.h"

#include "../DirectXTK12/Inc/DDSTextureLoader.h"
#include "../DirectXTK12/Inc/DirectXHelpers.h"
#include "../DirectXTK12/Inc/WICTextureLoader.h"

namespace DX12
{

	CDX12Resource::CDX12Resource(CDX12Engine* engine)
	{
		mEngine = engine;
	}

	CDX12Resource::CDX12Resource(CDX12Engine* engine, ComPtr<ID3D12Resource> r)
	{
		mEngine = engine;
		r.Swap(mResource);
	}

	void CDX12Resource::Barrier(D3D12_RESOURCE_STATES after)
	{
		if (after == mCurrentResourceState) return;

		const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			mResource.Get(),
			mCurrentResourceState,
			after);

		mEngine->mCurrRecordingCommandList->ResourceBarrier(1, &barrier);

		mCurrentResourceState = after;
	}

	CDX12Texture::CDX12Texture(CDX12Engine* engine, CDX12DescriptorHeap* srvHeap) : CDX12Resource(engine)
	{
		mSrvHandle = srvHeap->Add();
		mSrvHeap = srvHeap;
	}

	CDX12Texture::CDX12Texture(CDX12Engine* engine, std::string& filename, CDX12DescriptorHeap* srvHeap) : CDX12Resource(engine)
	{
		mSrvHandle = srvHeap->Add();
		mSrvHeap = srvHeap;

		LoadTexture(filename);

		mDesc = mResource->GetDesc();

		// Needed for raytracing
		mTextureRes = { mDesc.Width,mDesc.Height };
	}

	CDX12Texture::CDX12Texture(CDX12Engine* engine, D3D12_RESOURCE_DESC desc, CDX12DescriptorHeap* srvHeap) : CDX12Resource(engine)
	{
		mSrvHandle = srvHeap->Add();
		mSrvHeap = srvHeap;

		CreateTexture(desc);
	}

	CDX12Texture::~CDX12Texture()
	{
		if (mSrvHeap && mSrvHandle) mSrvHeap->Remove(mSrvHandle);
	}

	CDX12Texture::CDX12Texture(CDX12Engine* engine, const ComPtr<ID3D12Resource>& res) : CDX12Resource(engine, res)
	{
	}

	void CDX12Texture::Set(UINT rootParameterIndex)
	{
		auto handle = mSrvHeap->Get(mSrvHandle).mGpu;
		mEngine->mCurrRecordingCommandList->SetGraphicsRootDescriptorTable(rootParameterIndex, handle);
	}

	SHandle CDX12Texture::GetHandle()
	{
		return mSrvHeap->Get(mSrvHandle);
	}

	void CDX12Texture::LoadTexture(std::string& filename)
	{
		filename = mEngine->GetMediaFolder() + filename;

		const auto device = mEngine->mDevice.Get();

		DirectX::ResourceUploadBatch resourceUpload(device);

		resourceUpload.Begin();

		ComPtr<ID3D12Resource> textureResource;

		const auto resourceFlags = D3D12_RESOURCE_FLAG_NONE;

		const bool isSrgb = 0;
		//filename.find("Albedo") != std::string::npos;

		auto ddsFlags = (isSrgb ? DirectX::DDS_LOADER_FORCE_SRGB : DirectX::DDS_LOADER_DEFAULT) | DirectX::DDS_LOADER_MIP_AUTOGEN;
		auto wicFlags = (isSrgb ? DirectX::WIC_LOADER_FORCE_SRGB : DirectX::WIC_LOADER_DEFAULT) | DirectX::WIC_LOADER_MIP_AUTOGEN;

		// DDS files need a different function from other files
		std::string dds = ".dds"; // So check the filename extension (case insensitive)
		if (filename.size() >= 4 && std::equal(dds.rbegin(),
			dds.rend(),
			filename.rbegin(),
			[](unsigned char a, unsigned char b)
			{
				return std::tolower(a) == std::tolower(b);
			}))
		{
			if (FAILED(DirectX::CreateDDSTextureFromFileEx(
				device, resourceUpload, std::wstring(filename.begin(), filename.end()).c_str(), 0, resourceFlags, ddsFlags, textureResource.GetAddressOf())))
			{
				throw std::runtime_error("Failed to load image: " + filename);
			}
		}
		else
		{
			if (FAILED(DirectX::CreateWICTextureFromFileEx(
				device, resourceUpload, std::wstring(filename.begin(), filename.end()).c_str(), 0, resourceFlags, wicFlags, textureResource.GetAddressOf())))
			{
				throw std::runtime_error("Failed to load image: " + filename);
			}
		}

		DirectX::CreateShaderResourceView(device, textureResource.Get(), mSrvHeap->Get(mSrvHandle).mCpu);

		const auto uploadResourceFinished = resourceUpload.End(mEngine->mCommandQueue.Get());

		uploadResourceFinished.wait();

		mResource = textureResource;

		device->Release();
	}

	void CDX12Texture::CreateTexture(D3D12_RESOURCE_DESC desc)
	{
		constexpr FLOAT clearColor[4] = { 0.4f,0.6f,0.9f,1.0f };

		const auto clearValue = CD3DX12_CLEAR_VALUE(desc.Format, clearColor);

		const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		const auto device = mEngine->mDevice.Get();

		device->CreateCommittedResource(&heapProperties,
			D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&desc,
			D3D12_RESOURCE_STATE_COMMON,
			&clearValue,
			IID_PPV_ARGS(mResource.GetAddressOf()));


		mCurrentResourceState = D3D12_RESOURCE_STATE_COMMON;

		DirectX::CreateShaderResourceView(device,
			mResource.Get(),
			mSrvHeap->Get(mSrvHandle).mCpu,
			desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D);

		device->Release();
	}


	void CDX12Texture::CreateTexture(D3D12_RESOURCE_DESC desc, D3D12_CLEAR_VALUE clearValue)
	{
		const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		const auto device = mEngine->mDevice.Get();

		device->CreateCommittedResource(&heapProperties,
			D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&desc,
			D3D12_RESOURCE_STATE_COMMON,
			&clearValue,
			IID_PPV_ARGS(mResource.GetAddressOf()));


		mCurrentResourceState = D3D12_RESOURCE_STATE_COMMON;

		DirectX::CreateShaderResourceView(device,
			mResource.Get(),
			mSrvHeap->Get(mSrvHandle).mCpu,
			desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D);

		device->Release();
	}


	CDX12RenderTarget::CDX12RenderTarget(CDX12Engine* engine, ComPtr<ID3D12Resource> r, CDX12DescriptorHeap* rtvHeap)
		:
		CDX12Texture(engine, std::move(r))
	{
		mRTVHandle = rtvHeap->Add();
		mRtvHeap = rtvHeap;

		const auto device = mEngine->mDevice.Get();
		device->CreateRenderTargetView(mResource.Get(), nullptr, mRtvHeap->Get(mRTVHandle).mCpu);
		device->Release();
	}

	CDX12RenderTarget::CDX12RenderTarget(CDX12Engine* engine,
		D3D12_RESOURCE_DESC  desc,
		CDX12DescriptorHeap* srvHeap,
		CDX12DescriptorHeap* rtvHeap)
		:
		CDX12Texture(engine, desc, srvHeap)
	{
		mRTVHandle = rtvHeap->Add();
		mRtvHeap = rtvHeap;

		const auto device = mEngine->mDevice.Get();
		device->CreateRenderTargetView(mResource.Get(), nullptr, mRtvHeap->Get(mRTVHandle).mCpu);
		device->Release();
	}

	CDX12RenderTarget::~CDX12RenderTarget()
	{
		if (mRtvHeap) mRtvHeap->Remove(mRTVHandle);
	}

	CDX12DepthStencil::~CDX12DepthStencil()
	{
		if (mDsvHeap) mDsvHeap->Remove(mDsvHandle);
	}

	CDX12DepthStencil::CDX12DepthStencil(CDX12Engine* engine,
		const D3D12_RESOURCE_DESC& desc,
		CDX12DescriptorHeap* srvHeap,
		CDX12DescriptorHeap* dsvHeap)
		:
		CDX12Texture(engine, srvHeap)
	{
		mDsvHandle = dsvHeap->Add();
		mDsvHeap = dsvHeap;

		D3D12_CLEAR_VALUE clearValue;
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		const auto device = mEngine->mDevice.Get();

		device->CreateCommittedResource(&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COMMON,
			&clearValue,
			IID_PPV_ARGS(mResource.GetAddressOf()));

		device->CreateDepthStencilView(mResource.Get(), nullptr, mDsvHeap->Get(mDsvHandle).mCpu);

		device->Release();
	}
}
