#pragma once

#include "DX12Common.h"

namespace DX12
{
	class CDX12DescriptorHeap;
	class CDX12Engine;


	class CDX12Resource
	{
		public:

			virtual ~CDX12Resource() = default;

			CDX12Resource(CDX12Engine* engine);

			CDX12Resource(CDX12Engine* engine, ComPtr<ID3D12Resource> r);

		void Barrier(D3D12_RESOURCE_STATES after);

		CDX12Engine*               mEngine;
		ComPtr<ID3D12Resource>	   mResource;
		D3D12_RESOURCE_STATES      mCurrentResourceState = D3D12_RESOURCE_STATE_COMMON;
	};


	class CDX12Texture : public CDX12Resource
	{
	public:
			~CDX12Texture() override;

		CDX12Texture() = delete;
		CDX12Texture(const CDX12Texture&) = delete;
		CDX12Texture(const CDX12Texture&&) = delete;
		CDX12Texture(CDX12Engine* engine, const ComPtr<ID3D12Resource>& res);
		CDX12Texture& operator=(const CDX12Texture&) = delete;
		CDX12Texture& operator=(const CDX12Texture&&) = delete;

		SHandle*             mSrvHandle;
		CDX12DescriptorHeap* mSrvHeap;

		// Leave the resource uninitialized (use carefully)
		CDX12Texture(CDX12Engine* engine, CDX12DescriptorHeap* srvHeap);

		CDX12Texture(CDX12Engine* engine, std::string& filename, CDX12DescriptorHeap* srvHeap);

		CDX12Texture(CDX12Engine* engine, D3D12_RESOURCE_DESC desc, CDX12DescriptorHeap* srvHeap);

		void Set(UINT rootParameterIndex);

	protected:

		void LoadTexture(std::string& filename);
		void CreateTexture(D3D12_RESOURCE_DESC desc);
		void CreateTexture(D3D12_RESOURCE_DESC desc, D3D12_CLEAR_VALUE clearValue);
	};


	class CDX12RenderTarget : public CDX12Texture
	{
	public:
		~CDX12RenderTarget() override;

		CDX12RenderTarget() = delete;
		CDX12RenderTarget(const CDX12RenderTarget&) = delete;
		CDX12RenderTarget(const CDX12RenderTarget&&) = delete;
		CDX12RenderTarget& operator=(const CDX12RenderTarget&) = delete;
		CDX12RenderTarget& operator=(const CDX12RenderTarget&&) = delete;

		CDX12RenderTarget(CDX12Engine* engine, ComPtr<ID3D12Resource> r, CDX12DescriptorHeap* rtvHeap);
		CDX12RenderTarget(CDX12Engine* engine,
			D3D12_RESOURCE_DESC  desc,
			CDX12DescriptorHeap* srvHeap,
			CDX12DescriptorHeap* rtvHeap);


		SHandle* mRTVHandle;
		CDX12DescriptorHeap* mRtvHeap;
	};


	class CDX12DepthStencil : public CDX12Texture
	{
	public:

		virtual ~CDX12DepthStencil() override;

		CDX12DepthStencil(CDX12Engine* engine,
			const D3D12_RESOURCE_DESC& desc,
			CDX12DescriptorHeap* srvHeap,
			CDX12DescriptorHeap* dsvHeap);

		SHandle*             mDsvHandle;
		CDX12DescriptorHeap* mDsvHeap;
	};


	class CDX12TextureCube
	{
	public:

		virtual ~CDX12TextureCube();

		CDX12TextureCube() = delete;
		CDX12TextureCube(const CDX12TextureCube&) = delete;
		CDX12TextureCube(const CDX12TextureCube&&) = delete;
		CDX12TextureCube& operator=(const CDX12TextureCube&) = delete;
		CDX12TextureCube& operator=(const CDX12TextureCube&&) = delete;

		// Constructors

		// Leave the resource uninitialized (use carefully)
		CDX12TextureCube(CDX12Engine* engine, CDX12DescriptorHeap* heap);

		CDX12TextureCube(CDX12Engine* engine, ComPtr<ID3D12Resource> r);

		CDX12TextureCube(CDX12Engine* engine, std::string& filename, CDX12DescriptorHeap* heap);

		CDX12TextureCube(CDX12Engine* engine, D3D12_RESOURCE_DESC desc, CDX12DescriptorHeap* heap);


		// Usage

		void Barrier(D3D12_RESOURCE_STATES after);

		void Set(UINT rootParameterIndex);

		CDX12Engine* mPtrEngine;
		ComPtr<ID3D12Resource>  mResource;
		SHandle*   mHandle;
		D3D12_RESOURCE_STATES mCurrentResourceState;

	protected:
		void LoadTexture(std::string& filename);

		void CreateTexture(D3D12_RESOURCE_DESC desc);
		void CreateTexture(D3D12_RESOURCE_DESC desc, D3D12_CLEAR_VALUE   clearValue);
	};
}