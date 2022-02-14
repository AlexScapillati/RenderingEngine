#pragma once
#include <deque>

#include "DX12Engine.h"

class CDX12DescriptorHeap
{

public:

	CDX12DescriptorHeap(CDX12Engine* engine, D3D12_DESCRIPTOR_HEAP_DESC desc);

	CDX12DescriptorHeap() = delete;
	CDX12DescriptorHeap(const CDX12DescriptorHeap&) = delete;
	CDX12DescriptorHeap(const CDX12DescriptorHeap&&) = delete;
	CDX12DescriptorHeap& operator=(const CDX12DescriptorHeap&) = delete;
	CDX12DescriptorHeap& operator=(const CDX12DescriptorHeap&&) = delete;


	ComPtr<ID3D12DescriptorHeap> mDescriptorHeap;

	D3D12_DESCRIPTOR_HEAP_DESC GetDesc() const;

	SHandle Add();

	SHandle Get(INT pos) const;

	void Set() const;

	INT Top() const;

private:

	CDX12Engine* mEngine;
	D3D12_DESCRIPTOR_HEAP_DESC mDesc;
	INT mSize;
	INT mTop;


	std::deque<SHandle> mHandles;
};

