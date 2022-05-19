#include "DX12DescriptorHeap.h"

#include "DX12Engine.h"

namespace DX12
{
	CDX12DescriptorHeap::~CDX12DescriptorHeap()
	{
		// Remove all the handles
		ZeroMemory(this, sizeof * this);
	}

	CDX12DescriptorHeap::CDX12DescriptorHeap(CDX12Engine* engine, D3D12_DESCRIPTOR_HEAP_DESC desc) :
		mSize(desc.NumDescriptors),
		mNextFree(0),
		mEngine(engine),
		mDesc(desc)
	{
		mFreeIndexes.resize(mSize);

		for (uint32_t i = 0; i < mSize; ++i)
		{
			mFreeIndexes[i] = i;
		}

		if (FAILED(mEngine->mDevice->CreateDescriptorHeap(&mDesc, IID_PPV_ARGS(&mDescriptorHeap))))
		{
			throw std::runtime_error("Error Creating Descriptor Heap");
		}

		mIncrementSize = mEngine->mDevice->GetDescriptorHandleIncrementSize(mDesc.Type);
	}

	CDX12DescriptorHeap::CDX12DescriptorHeap(CDX12Engine* engine,
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		UINT count,
		D3D12_DESCRIPTOR_HEAP_FLAGS flags) :
		mSize(count),
		mNextFree(0),
		mEngine(engine)
	{
		mDesc.NumDescriptors = count;
		mDesc.Flags = flags;
		mDesc.Type = type;

		mFreeIndexes.resize(mSize);

		for (uint32_t i = 0; i < mSize; ++i)
		{
			mFreeIndexes[i] = i;
		}

		if (FAILED(mEngine->mDevice->CreateDescriptorHeap(&mDesc, IID_PPV_ARGS(&mDescriptorHeap))))
		{
			throw std::runtime_error("Error Creating Descriptor Heap");
		}

		mIncrementSize = mEngine->mDevice->GetDescriptorHandleIncrementSize(mDesc.Type);
	}

	D3D12_DESCRIPTOR_HEAP_DESC CDX12DescriptorHeap::GetDesc() const
	{
		return mDesc;
	}

	uint32_t CDX12DescriptorHeap::Add()
	{
		if (mNextFree == mSize) throw std::bad_alloc();

		auto freeIndex = mFreeIndexes[mNextFree];
		mNextFree++;

		return freeIndex;
	}

	SHandle CDX12DescriptorHeap::Get(UINT pos)
	{
		if (pos == mSize) throw std::out_of_range("Out of range");

		SHandle newHandle;
		newHandle.mIndexInDescriptor = pos;
		newHandle.mCpu = CD3DX12_CPU_DESCRIPTOR_HANDLE(mDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), newHandle.mIndexInDescriptor, mIncrementSize);
		newHandle.mGpu = mDesc.Flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE ?
			CD3DX12_GPU_DESCRIPTOR_HANDLE(mDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), newHandle.mIndexInDescriptor, mIncrementSize) :
			CD3DX12_GPU_DESCRIPTOR_HANDLE();
		return newHandle;
	}

	void CDX12DescriptorHeap::Remove(UINT pos)
	{
		if (pos == mSize) throw std::out_of_range("Out of range");
		--mNextFree;
		mFreeIndexes[mNextFree] = pos;
	}

	void CDX12DescriptorHeap::Set() const
	{
		ID3D12DescriptorHeap* const pheap[] = { mDescriptorHeap.Get() };
		mEngine->mCurrRecordingCommandList->SetDescriptorHeaps(1, pheap);
	}
}