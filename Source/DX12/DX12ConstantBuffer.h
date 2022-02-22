#pragma once

#include "DX12Engine.h"
#include "DX12DescriptorHeap.h"

namespace DX12
{
	class CDX12ConstantBuffer
	{
	public:

		CDX12ConstantBuffer() = delete;
		CDX12ConstantBuffer(const CDX12ConstantBuffer&) = delete;
		CDX12ConstantBuffer(const CDX12ConstantBuffer&&) = delete;
		CDX12ConstantBuffer& operator=(const CDX12ConstantBuffer&) = delete;
		CDX12ConstantBuffer& operator=(const CDX12ConstantBuffer&&) = delete;

		CDX12ConstantBuffer(CDX12Engine* engine, size_t size) :
			mEngine(engine),
			mSize(size)
		{

			const auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

			const auto buffer = CD3DX12_RESOURCE_DESC::Buffer(size);

			ThrowIfFailed(
				mEngine->mDevice->CreateCommittedResource(
					&prop,
					D3D12_HEAP_FLAG_NONE,
					&buffer,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&mResource)));

			// Describe and create a constant buffer view.
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = mResource->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = static_cast<UINT>(size);

			mDescriptorIndex = mEngine->mCBVDescriptorHeap->Top();

			mHandle = mEngine->mCBVDescriptorHeap->Add();

			mEngine->mDevice->CreateConstantBufferView(&cbvDesc, mHandle.mCpu);

			// Map and initialize the constant buffer. We don't unmap this until the
			// app closes. Keeping things mapped for the lifetime of the resource is okay.
			const CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
			ThrowIfFailed(mResource->Map(0, &readRange, reinterpret_cast<void**>(&mCBVDataBegin)));
			mResource->Unmap(0, nullptr);
		}


		template <typename T>
		void Copy(T& data) const
		{
			memcpy(mCBVDataBegin, &data, mSize);
		}

		void Set(UINT RootParameterIndex) const
		{
			mEngine->mCommandList->SetGraphicsRootDescriptorTable(RootParameterIndex, mHandle.mGpu);
		}

	private:

		CDX12Engine* mEngine;
		ComPtr<ID3D12Resource> mResource;
		UINT8* mCBVDataBegin;
		size_t mSize;

		SHandle mHandle;

		INT mDescriptorIndex;
	};
}