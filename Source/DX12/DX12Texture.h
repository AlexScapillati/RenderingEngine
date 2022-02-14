#pragma once

#include "CDX12Common.h"

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
	DX12Common::Resource  mResource;
	DX12Common::SHandle   mHandle;
	INT                   mDescriptorIndex;
	D3D12_RESOURCE_STATES mCurrentResourceState;

	CDX12Texture(CDX12Engine* engine, DX12Common::Resource r);

	CDX12Texture(CDX12Engine* engine, std::string& filename);

	CDX12Texture(CDX12Engine* engine, D3D12_RESOURCE_DESC desc);

	virtual ~CDX12Texture();

	virtual void Reset(D3D12_RESOURCE_DESC desc);

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

	CDX12RenderTarget(CDX12Engine* engine, DX12Common::Resource r);

	CDX12RenderTarget(CDX12Engine* engine, D3D12_RESOURCE_DESC desc);

	~CDX12RenderTarget() override;

	void Reset(D3D12_RESOURCE_DESC desc) override;

	DX12Common::SHandle mRTVHandle;

	INT mRTVDescriptorIndex;


};
