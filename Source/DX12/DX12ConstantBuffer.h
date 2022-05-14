#pragma once

#include "DX12Common.h"
#include "DX12DescriptorHeap.h"

namespace DX12
{
	class CDX12Engine;

	class CDX12ConstantBuffer
	{
	public:

		CDX12ConstantBuffer()                                       = delete;
		CDX12ConstantBuffer(const CDX12ConstantBuffer&)             = delete;
		CDX12ConstantBuffer(const CDX12ConstantBuffer&&)            = delete;
		CDX12ConstantBuffer& operator=(const CDX12ConstantBuffer&)  = delete;
		CDX12ConstantBuffer& operator=(const CDX12ConstantBuffer&&) = delete;
		UINT                  Size();

		CDX12ConstantBuffer(CDX12Engine* engine, CDX12DescriptorHeap* cbvHeap, size_t size);

		template <typename T>
		void Copy(T& data)
		{
			// Map and initialize the constant buffer. 
			const CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
			ThrowIfFailed(mResource->Map(0, &readRange, reinterpret_cast<void**>(&mCBVDataBegin)));
			if (mCBVDataBegin)
			{
				memcpy(mCBVDataBegin, &data, mSize);
				mResource->Unmap(0, nullptr);
			}
		}

		template <typename T, typename U>
		void Copy(T& data, size_t n)
		{
			// Map and initialize the constant buffer. 
			const CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
			ThrowIfFailed(mResource->Map(0, &readRange, reinterpret_cast<void**>(&mCBVDataBegin)));
			memcpy(mCBVDataBegin, &data, sizeof(U) * n);
			mResource->Unmap(0, nullptr);
		}

		void Set(UINT RootParameterIndex) const;

		auto Resource() const { return mResource; }

	private:
		CDX12Engine*           mEngine;
		ComPtr<ID3D12Resource> mResource;
		UINT8*                 mCBVDataBegin;
		size_t                 mSize;
		CDX12DescriptorHeap*   mCBVHeap;

		SHandle* mHandle;
	};
}
