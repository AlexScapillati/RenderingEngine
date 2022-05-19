#include "DX12ConstantBuffer.h"
#include "DX12Engine.h"
#include "DXR/DXR.h"

UINT DX12::CDX12ConstantBuffer::Size()
{
	return mSize;
}

DX12::CDX12ConstantBuffer::CDX12ConstantBuffer(CDX12Engine* engine, CDX12DescriptorHeap* cbvHeap, size_t size) :
	mEngine(engine),
	mSize(size)
{
	mCBVHeap = cbvHeap;

	const auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	mSize = ROUND_UP(size,256);

	const auto buffer = CD3DX12_RESOURCE_DESC::Buffer(mSize);
	
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
	cbvDesc.SizeInBytes = static_cast<UINT>(mSize);

	mHandle = mCBVHeap->Add();

	mEngine->mDevice->CreateConstantBufferView(&cbvDesc, mCBVHeap->Get(mHandle).mCpu);

	// Map and initialize the constant buffer. 
	const CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
	ThrowIfFailed(mResource->Map(0, &readRange, reinterpret_cast<void**>(&mCBVDataBegin)));
	mResource->Unmap(0, nullptr);
}


void DX12::CDX12ConstantBuffer::Set(UINT RootParameterIndex) const
{
	mEngine->mCurrRecordingCommandList->SetGraphicsRootDescriptorTable(RootParameterIndex, mCBVHeap->Get(mHandle).mGpu);
}
