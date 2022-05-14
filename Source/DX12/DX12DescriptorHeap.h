#pragma once

#include <mutex>

#include "DX12Common.h"

namespace DX12
{
	class CDX12Engine;

	class CDX12DescriptorHeap
	{

	public:

		virtual ~CDX12DescriptorHeap();

		CDX12DescriptorHeap(CDX12Engine* engine, D3D12_DESCRIPTOR_HEAP_DESC desc);

		explicit CDX12DescriptorHeap(CDX12Engine*                engine,
									 D3D12_DESCRIPTOR_HEAP_TYPE  type  = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
									 UINT                        count = 1,
									 D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

		CDX12DescriptorHeap() = delete;
		CDX12DescriptorHeap(const CDX12DescriptorHeap&) = delete;
		CDX12DescriptorHeap(const CDX12DescriptorHeap&&) = delete;
		CDX12DescriptorHeap& operator=(const CDX12DescriptorHeap&) = delete;
		CDX12DescriptorHeap& operator=(const CDX12DescriptorHeap&&) = delete;

		ComPtr<ID3D12DescriptorHeap> mDescriptorHeap;

		D3D12_DESCRIPTOR_HEAP_DESC GetDesc() const;

		uint32_t Add();

		SHandle* Get(UINT pos);

		void Remove(UINT pos);

		void Set() const;

		std::vector<SHandle> mHandles;

	private:
		
		CDX12Engine* mEngine;
		D3D12_DESCRIPTOR_HEAP_DESC mDesc;
		UINT mIncrementSize;

	};

}