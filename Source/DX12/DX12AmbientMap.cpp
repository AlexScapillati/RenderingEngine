#include "DX12AmbientMap.h"

#include "DX12Engine.h"
#include "../Common/CGameObject.h"
#include "../Common/CGameObjectManager.h"
#include "DX12ConstantBuffer.h"

namespace DX12
{
	CDX12AmbientMap::CDX12AmbientMap(CDX12Engine* e, int size, CDX12DescriptorHeap* rtvHeap, CDX12DescriptorHeap* srvHeap, CDX12DescriptorHeap* dsvHeap) :
		mEngine(e),
		mSize(size)
	{

		mVp = CD3DX12_VIEWPORT(0.f, 0.f, static_cast<float>(mSize), static_cast<float>(mSize));
		mScissorsRect = { 0,0,mSize,mSize };

		mSrvIndex = srvHeap->Top();
		mSrvHandle = srvHeap->Add();

		mDsvIndex = dsvHeap->Top();
		mDsvHandle = dsvHeap->Add();

		for (int i = 0; i < 6; ++i)
		{
			mRtvIndex[i] = rtvHeap->Top();
			mRtvHandle[i] = rtvHeap->Add();
		}


		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Alignment = 0;
		desc.Width = mSize;
		desc.Height = mSize;
		desc.DepthOrArraySize = 6;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		const float c[] = { 0.4f,0.6f,0.9f,1.0f };
		CD3DX12_CLEAR_VALUE clearValue(desc.Format, c);

		auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		ThrowIfFailed(
			mEngine->mDevice->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				&clearValue,
				IID_PPV_ARGS(mResource.GetAddressOf())));


		NAME_D3D12_OBJECT(mResource);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = 1;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.f;

		mEngine->mDevice->CreateShaderResourceView(mResource.Get(), &srvDesc, mSrvHandle.mCpu);

		for (int i = 0; i < 6; ++i)
		{
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
			rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Texture2DArray.ArraySize = 1; // Only view one element of the array
			rtvDesc.Texture2DArray.MipSlice = 0;
			rtvDesc.Texture2DArray.PlaneSlice = 0;
			rtvDesc.Texture2DArray.FirstArraySlice = i;

			mEngine->mDevice->CreateRenderTargetView(mResource.Get(), &rtvDesc, mRtvHandle[i].mCpu);
		}

		D3D12_RESOURCE_DESC dsvDesc{};
		dsvDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		dsvDesc.Alignment = 0;
		dsvDesc.Width = mSize;
		dsvDesc.Height = mSize;
		dsvDesc.DepthOrArraySize = 1;
		dsvDesc.MipLevels = 1;
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.SampleDesc.Count = 1;
		dsvDesc.SampleDesc.Quality = 0;
		dsvDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		dsvDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		

		auto cl = CD3DX12_CLEAR_VALUE(dsvDesc.Format,1.0f,0);

		ThrowIfFailed(
			mEngine->mDevice->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&dsvDesc,
				D3D12_RESOURCE_STATE_DEPTH_READ,
				&cl,
				IID_PPV_ARGS(mDepthBufferResource.GetAddressOf())));

		mEngine->mDevice->CreateDepthStencilView(mDepthBufferResource.Get(), nullptr, mDsvHandle.mCpu);

		NAME_D3D12_OBJECT(mDepthBufferResource);
	}

	void* CDX12AmbientMap::RenderFromThis(CMatrix4x4* mat)
	{

		mEngine->mCommandList->RSSetViewports(1, &mVp);
		mEngine->mCommandList->RSSetScissorRects(1, &mScissorsRect);

		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			mResource.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_RESOURCE_STATE_RENDER_TARGET);

		mEngine->mCommandList->ResourceBarrier(1, &barrier);

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			mDepthBufferResource.Get(),
			D3D12_RESOURCE_STATE_DEPTH_READ,
			D3D12_RESOURCE_STATE_DEPTH_WRITE);


		mEngine->mCommandList->ResourceBarrier(1, &barrier);


		float mSides[6][3] = {
			// Starting from facing down the +ve Z direction, left handed rotations
			{0.0f, 0.5f, 0.0f},  // +ve X direction (values multiplied by PI)
			{0.0f, -0.5f, 0.0f}, // -ve X direction
			{-0.5f, 0.0f, 0.0f}, // +ve Y direction
			{0.5f, 0.0f, 0.0f},  // -ve Y direction
			{0.0f, 0.0f, 0.0f},  // +ve Z direction
			{0.0f, 1.0f, 0.0f}   // -ve Z direction
		};


		auto scale = mat->GetScale();
		auto pos = mat->GetPosition();
		auto originalMatrix = *mat;

		for (int i = 0; i < 6; ++i)
		{
			CVector3 rotation = mSides[i];

			// To put rotation angles into a matrix we need to build the matrix from scratch to make sure we retain existing scaling and position
			*mat = MatrixScaling(scale) *
				MatrixRotationZ(rotation.z) * MatrixRotationX(rotation.x) * MatrixRotationY(rotation.y) *
				MatrixTranslation(pos);

			const FLOAT clearColor[] = { 0.4f,0.6f,0.9f,1.0f };
			mEngine->mCommandList->ClearRenderTargetView(mRtvHandle[i].mCpu, clearColor, 0, nullptr);
			mEngine->mCommandList->ClearDepthStencilView(mDsvHandle.mCpu, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

			mEngine->mCommandList->OMSetRenderTargets(1, &mRtvHandle[i].mCpu, true, &mDsvHandle.mCpu);

			mEngine->mPerFrameConstants.viewMatrix = InverseAffine(*mat);
			mEngine->mPerFrameConstants.projectionMatrix = MakeProjectionMatrix(1.0f, ToRadians(90.f));
			mEngine->mPerFrameConstants.viewProjectionMatrix = mEngine->mPerFrameConstants.viewMatrix * mEngine->mPerFrameConstants.projectionMatrix;

			mEngine->mPerFrameConstantBuffer->Copy(mEngine->mPerFrameConstants);


			mEngine->GetObjManager()->mSky->Render();

			for(auto& o : mEngine->GetObjManager()->mObjects)
			{
				o->Render();
			}
		}

		// restore original matrix
		*mat = originalMatrix;

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			mResource.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_GENERIC_READ);

		mEngine->mCommandList->ResourceBarrier(1, &barrier);

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			mDepthBufferResource.Get(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			D3D12_RESOURCE_STATE_DEPTH_READ);

		mEngine->mCommandList->ResourceBarrier(1, &barrier);


		return &mSrvHandle.mGpu;
	}



}
