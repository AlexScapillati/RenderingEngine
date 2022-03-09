#pragma once

#include "DX12Common.h"

namespace DX12
{
	class CDX12DescriptorHeap;
	class CDX12Engine;

	class CDX12Texture
	{
	public:

		CDX12Texture() = delete;
		CDX12Texture(const CDX12Texture&) = delete;
		CDX12Texture(const CDX12Texture&&) = delete;
		CDX12Texture& operator=(const CDX12Texture&) = delete;
		CDX12Texture& operator=(const CDX12Texture&&) = delete;


		CDX12Engine* mPtrEngine;
		Resource  mResource;
		SHandle   mHandle;
		INT                   mDescriptorIndex;
		D3D12_RESOURCE_STATES mCurrentResourceState;

		CDX12Texture(CDX12Engine* engine,Resource r);

		CDX12Texture(CDX12Engine* engine, std::string& filename, CDX12DescriptorHeap* heap);

		CDX12Texture(CDX12Engine* engine, D3D12_RESOURCE_DESC desc, CDX12DescriptorHeap* heap);
		
		virtual ~CDX12Texture();

		void Barrier(D3D12_RESOURCE_STATES after);

		void Set(UINT rootParameterIndex);


	protected:
		void LoadTexture(std::string& filename);

		void CreateTexture(D3D12_RESOURCE_DESC desc);
	};

	class CDX12RenderTarget final : public CDX12Texture
	{
	public:

		CDX12RenderTarget() = delete;
		CDX12RenderTarget(const CDX12RenderTarget&) = delete;
		CDX12RenderTarget(const CDX12RenderTarget&&) = delete;
		CDX12RenderTarget& operator=(const CDX12RenderTarget&) = delete;
		CDX12RenderTarget& operator=(const CDX12RenderTarget&&) = delete;

		CDX12RenderTarget(CDX12Engine* engine,Resource r);

		CDX12RenderTarget(CDX12Engine* engine, D3D12_RESOURCE_DESC desc,CDX12DescriptorHeap* heap);

		~CDX12RenderTarget() override;

		SHandle mRTVHandle{};

		INT mRTVDescriptorIndex;
	};
}