#include "DX12AmbientMap.h"

<<<<<<< HEAD
#include "DX12ConstantBuffer.h"
#include "DX12Engine.h"
#include "DX12PipelineObject.h"
#include "../Common/CGameObject.h"
#include "../Common/CScene.h"
#include "../Common/CGameObjectManager.h"

namespace DX12
{
	CDX12AmbientMap::CDX12AmbientMap(CDX12Engine* e, int size, CDX12DescriptorHeap* srvHeap) :
		mEngine(e),
		mSize(size)
	{
		mEnable = true;
		mVp = CD3DX12_VIEWPORT(0.f, 0.f, static_cast<float>(mSize), static_cast<float>(mSize));
		mScissorsRect = { 0,0,mSize,mSize };

		mSrvHeap = srvHeap;
		mSrvHandle = srvHeap->Add();

		mDsvHeap = std::make_unique<CDX12DescriptorHeap>(mEngine, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		mRtvHeap = std::make_unique<CDX12DescriptorHeap>(mEngine, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 6);

		mDsvHandle = mDsvHeap->Add();

		for (int i = 0; i < 6; ++i)
		{
			mRtvHandle[i] = mRtvHeap->Add();
		}
		
=======
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
		mEnable = false;
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


>>>>>>> parent of a9c1de14 (revert commit)
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

<<<<<<< HEAD

=======
>>>>>>> parent of a9c1de14 (revert commit)
		const float c[] = { 0.4f,0.6f,0.9f,1.0f };
		CD3DX12_CLEAR_VALUE clearValue(desc.Format, c);

		auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		ThrowIfFailed(
			mEngine->mDevice->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&desc,
<<<<<<< HEAD
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				&clearValue,
				IID_PPV_ARGS(mResource.GetAddressOf())));

		mResource->SetName(L"AmbientMap");

=======
				D3D12_RESOURCE_STATE_GENERIC_READ,
				&clearValue,
				IID_PPV_ARGS(mResource.GetAddressOf())));


		NAME_D3D12_OBJECT(mResource);
>>>>>>> parent of a9c1de14 (revert commit)

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = 1;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.f;

<<<<<<< HEAD
		mEngine->mDevice->CreateShaderResourceView(mResource.Get(), &srvDesc, mSrvHeap->Get(mSrvHandle)->mCpu);
=======
		mEngine->mDevice->CreateShaderResourceView(mResource.Get(), &srvDesc, mSrvHandle.mCpu);
>>>>>>> parent of a9c1de14 (revert commit)

		for (int i = 0; i < 6; ++i)
		{
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
			rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Texture2DArray.ArraySize = 1; // Only view one element of the array
			rtvDesc.Texture2DArray.MipSlice = 0;
			rtvDesc.Texture2DArray.PlaneSlice = 0;
			rtvDesc.Texture2DArray.FirstArraySlice = i;

<<<<<<< HEAD
			mEngine->mDevice->CreateRenderTargetView(mResource.Get(), &rtvDesc, mRtvHeap->Get(mRtvHandle[i])->mCpu);
=======
			mEngine->mDevice->CreateRenderTargetView(mResource.Get(), &rtvDesc, mRtvHandle[i].mCpu);
>>>>>>> parent of a9c1de14 (revert commit)
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
<<<<<<< HEAD


		auto cl = CD3DX12_CLEAR_VALUE(dsvDesc.Format, 1.0f, 0);
=======
		

		auto cl = CD3DX12_CLEAR_VALUE(dsvDesc.Format,1.0f,0);
>>>>>>> parent of a9c1de14 (revert commit)

		ThrowIfFailed(
			mEngine->mDevice->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&dsvDesc,
				D3D12_RESOURCE_STATE_DEPTH_READ,
				&cl,
				IID_PPV_ARGS(mDepthBufferResource.GetAddressOf())));

<<<<<<< HEAD
		mEngine->mDevice->CreateDepthStencilView(mDepthBufferResource.Get(), nullptr, mDsvHeap->Get(mDsvHandle)->mCpu);

		NAME_D3D12_OBJECT(mDepthBufferResource);


		for (int i = 0; i < 6; ++i)
		{
			mConstantBuffers[i] = std::make_unique<CDX12ConstantBuffer>(mEngine, mEngine->mSRVDescriptorHeap.get(), sizeof(PerFrameConstants));
		}
=======
		mEngine->mDevice->CreateDepthStencilView(mDepthBufferResource.Get(), nullptr, mDsvHandle.mCpu);

		NAME_D3D12_OBJECT(mDepthBufferResource);
>>>>>>> parent of a9c1de14 (revert commit)
	}

	void* CDX12AmbientMap::RenderFromThis(CMatrix4x4* mat)
	{
<<<<<<< HEAD
<<<<<<< HEAD
		if (!mEnable) return nullptr;

=======

		if (!mEnable) return nullptr;

		PIXBeginEvent(mEngine->mCommandList.Get(),0,L"AmbientMapRendering");
>>>>>>> parent of 20e675b8 (lab)

		// Reset all the other command allocators and command lists
		for (size_t i = 0; i < ARRAYSIZE(mEngine->mAmbientMapCommandLists); ++i)
		{
			auto j = i + mEngine->mCurrentBackBufferIndex * CDX12Engine::mNumFrames;
			auto commandAllocator = mEngine->mAmbientMapCommandAllocators[j].Get();
			commandAllocator->Reset();
			mEngine->mAmbientMapCommandLists[i]->Reset(commandAllocator, nullptr);
		}

		auto commandList = mEngine->mAmbientMapCommandLists[0].Get();
		mEngine->mCurrRecordingCommandList = commandList;

		PIXBeginEvent(commandList, 0, L"AmbientMapRendering");
=======

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
>>>>>>> parent of a9c1de14 (revert commit)


		float mSides[6][3] = {
			// Starting from facing down the +ve Z direction, left handed rotations
			{0.0f, 0.5f, 0.0f},  // +ve X direction (values multiplied by PI)
			{0.0f, -0.5f, 0.0f}, // -ve X direction
			{-0.5f, 0.0f, 0.0f}, // +ve Y direction
			{0.5f, 0.0f, 0.0f},  // -ve Y direction
			{0.0f, 0.0f, 0.0f},  // +ve Z direction
			{0.0f, 1.0f, 0.0f}   // -ve Z direction
		};


<<<<<<< HEAD
		PrepareToRender();

=======
>>>>>>> parent of a9c1de14 (revert commit)
		auto scale = mat->GetScale();
		auto pos = mat->GetPosition();
		auto originalMatrix = *mat;

		for (int i = 0; i < 6; ++i)
		{
<<<<<<< HEAD
			commandList = mEngine->mAmbientMapCommandLists[i].Get();
			mEngine->mCurrRecordingCommandList = commandList;

			auto rotation = CVector3(mSides[i]) * PI;

			// To put rotation angles into a matrix we need to build the matrix from scratch to make sure we retain existing scaling and position
			*mat =
				MatrixScaling(scale) *
				MatrixRotationZ(rotation.z) * MatrixRotationX(rotation.x) * MatrixRotationY(rotation.y) *
				MatrixTranslation(pos);

			constexpr FLOAT clearColor[] = { 0.4f,0.6f,0.9f,1.0f };

			CCamera camera(pos, rotation, PI, 1);

			mEngine->mCurrSetPso = nullptr;
			mEngine->SetPBRPSO();

			commandList->RSSetViewports(1, &mVp);
			commandList->RSSetScissorRects(1, &mScissorsRect);
			commandList->OMSetRenderTargets(1, &mRtvHeap->Get(mRtvHandle[i])->mCpu, false, &mDsvHeap->Get(mDsvHandle)->mCpu);
			commandList->ClearDepthStencilView(mDsvHeap->Get(mDsvHandle)->mCpu, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
			commandList->ClearRenderTargetView(mRtvHeap->Get(mRtvHandle[i])->mCpu, clearColor, 0, nullptr);

			// Set camera matrices in the constant buffer and send over to GPU
			mEngine->mSRVDescriptorHeap->Set();

			PerFrameConstants perFrameConstants;
			perFrameConstants.cameraMatrix = camera.WorldMatrix();
			perFrameConstants.viewMatrix = camera.ViewMatrix();
			perFrameConstants.projectionMatrix = camera.ProjectionMatrix();
			perFrameConstants.viewProjectionMatrix = camera.ViewProjectionMatrix();

			mConstantBuffers[i]->Copy(perFrameConstants);
			mConstantBuffers[i]->Set(2);

			mEngine->GetObjManager()->mSky->Render();

			for (auto object : mEngine->GetObjManager()->mObjects)
			{
				object->Render();
			}

			commandList->Close();

			ID3D12CommandList* cm[] = { commandList };
			mEngine->mCommandQueue->ExecuteCommandLists(1, cm);
			mEngine->mCurrSetPso = nullptr;
		}


		mEngine->mCurrRecordingCommandList = mEngine->GetCommandList();

		// restore original matrix
		*mat = originalMatrix;

		PrepareToShow();

		PIXEndEvent(mEngine->mCurrRecordingCommandList);

		return &mSrvHeap->Get(mSrvHandle)->mGpu;
	}
	void CDX12AmbientMap::PrepareToRender()
	{
		std::vector<D3D12_RESOURCE_BARRIER> v
		{ {CD3DX12_RESOURCE_BARRIER::Transition(
			mResource.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET)},
			{CD3DX12_RESOURCE_BARRIER::Transition(
			mDepthBufferResource.Get(),
			D3D12_RESOURCE_STATE_DEPTH_READ,
			D3D12_RESOURCE_STATE_DEPTH_WRITE)} };

		mEngine->mCurrRecordingCommandList->ResourceBarrier(v.size(), v.data());

	}

	void CDX12AmbientMap::PrepareToShow()
	{

		std::vector<D3D12_RESOURCE_BARRIER> v;

		v.push_back(CD3DX12_RESOURCE_BARRIER::Transition(
			mResource.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));


		v.push_back(CD3DX12_RESOURCE_BARRIER::Transition(
			mDepthBufferResource.Get(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			D3D12_RESOURCE_STATE_DEPTH_READ));

		mEngine->mCurrRecordingCommandList->ResourceBarrier(v.size(), v.data());
	}
=======
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



>>>>>>> parent of a9c1de14 (revert commit)
}
