#include "DX12DescriptorHeap.h"

namespace DX12
{

	CDX12DescriptorHeap::CDX12DescriptorHeap(CDX12Engine* engine, D3D12_DESCRIPTOR_HEAP_DESC desc) :
		mEngine(engine),
		mDesc(desc),
		mTop(0)
	{
		if (FAILED(mEngine->mDevice->CreateDescriptorHeap(&mDesc, IID_PPV_ARGS(&mDescriptorHeap))))
		{
			throw std::runtime_error("Error Creating Descriptor Heap");
		}

		mSize = mEngine->mDevice->GetDescriptorHandleIncrementSize(mDesc.Type);
	}

	D3D12_DESCRIPTOR_HEAP_DESC CDX12DescriptorHeap::GetDesc() const
	{
		return mDesc;
	}

	SHandle CDX12DescriptorHeap::Add()
	{
		const SHandle newHandle
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE(mDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),mTop,mSize),
			mDesc.Flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE ?
				CD3DX12_GPU_DESCRIPTOR_HANDLE(mDescriptorHeap->GetGPUDescriptorHandleForHeapStart(),mTop,mSize) :
				CD3DX12_GPU_DESCRIPTOR_HANDLE()
		};

		mTop++;

		mHandles.push_back(newHandle);
		return mHandles.back();
	}

	SHandle CDX12DescriptorHeap::Get(UINT pos) const
	{
		return mHandles[pos];
	}

	void CDX12DescriptorHeap::Set() const
	{
		ID3D12DescriptorHeap* const pheap[] = { mDescriptorHeap.Get() };
		mEngine->mCommandList->SetDescriptorHeaps(1, pheap);
	}

	INT CDX12DescriptorHeap::Top() const
	{
		return mTop;
	}
}