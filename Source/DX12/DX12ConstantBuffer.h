#pragma once

#include "DX12Common.h"

namespace DX12
{
	class CDX12Engine;

	class CDX12ConstantBuffer
	{
	public:

		CDX12ConstantBuffer() = delete;
		CDX12ConstantBuffer(const CDX12ConstantBuffer&) = delete;
		CDX12ConstantBuffer(const CDX12ConstantBuffer&&) = delete;
		CDX12ConstantBuffer& operator=(const CDX12ConstantBuffer&) = delete;
		CDX12ConstantBuffer& operator=(const CDX12ConstantBuffer&&) = delete;

		CDX12ConstantBuffer(CDX12Engine* engine, size_t size);

		template <typename T>
		void Copy(T& data) const
		{
			memcpy(mCBVDataBegin, &data, mSize);
		}

		template <typename T, typename U>
		void Copy(T& data, size_t n)
		{
			memcpy(mCBVDataBegin, &data, sizeof(U) * n);
		}

		void Set(UINT RootParameterIndex) const;

	private:
		CDX12Engine*           mEngine;
		ComPtr<ID3D12Resource> mResource;
		UINT8*                 mCBVDataBegin;
		size_t                 mSize;

		SHandle mHandle;

		INT mDescriptorIndex;
	};
}
