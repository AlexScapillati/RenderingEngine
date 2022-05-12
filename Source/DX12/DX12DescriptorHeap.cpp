#include "DX12DescriptorHeap.h"

#include "DX12Engine.h"

namespace DX12
{
	CDX12DescriptorHeap::~CDX12DescriptorHeap()
	{
		// Remove all the handles
	}

	CDX12DescriptorHeap::CDX12DescriptorHeap(CDX12Engine* engine, D3D12_DESCRIPTOR_HEAP_DESC desc) :
		mEngine(engine),
		mDesc(desc)
	{

		mHandles.reserve(desc.NumDescriptors);

		if (FAILED(mEngine->mDevice->CreateDescriptorHeap(&mDesc, IID_PPV_ARGS(&mDescriptorHeap))))
		{
			throw std::runtime_error("Error Creating Descriptor Heap");
		}

		mIncrementSize = mEngine->mDevice->GetDescriptorHandleIncrementSize(mDesc.Type);
	}

	CDX12DescriptorHeap::CDX12DescriptorHeap(CDX12Engine* engine,
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		UINT count,
		D3D12_DESCRIPTOR_HEAP_FLAGS flags)
	{
		mEngine = engine;
		mDesc.NumDescriptors = count;
		mDesc.Flags = flags;
		mDesc.Type = type;

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
		SHandle newHandle;
		newHandle.mIndexInDescriptor = static_cast<INT>(mHandles.size());
		newHandle.mCpu = CD3DX12_CPU_DESCRIPTOR_HANDLE(mDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), newHandle.mIndexInDescriptor, mIncrementSize);
		newHandle.mGpu = mDesc.Flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE ?
			CD3DX12_GPU_DESCRIPTOR_HANDLE(mDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), newHandle.mIndexInDescriptor, mIncrementSize) :
			CD3DX12_GPU_DESCRIPTOR_HANDLE();

		mHandles.push_back(newHandle);
		return newHandle.mIndexInDescriptor;
	}

	SHandle* CDX12DescriptorHeap::Get(UINT pos)
	{
		return &mHandles[pos];
	}

	void CDX12DescriptorHeap::Remove(UINT pos)
	{
		// TODO
		/*
		if (pos < mHandles.size())
		{
			mHandles.erase(mHandles.begin() + pos);

			for (int i = 0; i < mHandles.size(); ++i)
			{
				mHandles[i].mIndexInDescriptor = i;
			}
		}
		*/
	}

	void CDX12DescriptorHeap::Set() const
	{
		ID3D12DescriptorHeap* const pheap[] = { mDescriptorHeap.Get() };
		mEngine->mCurrRecordingCommandList->SetDescriptorHeaps(1, pheap);
	}
}