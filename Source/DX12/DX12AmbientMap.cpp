#include "DX12AmbientMap.h"

#include "DX12ConstantBuffer.h"
#include "DX12Engine.h"
#include "DX12PipelineObject.h"
#include "DX12Scene.h"
#include "DX12Texture.h"
#include "../Common/CGameObjectManager.h"
#include "../Common/Camera.h"
#include "Objects/CDX12Sky.h"

namespace DX12
{
	CDX12AmbientMap::CDX12AmbientMap(CDX12Engine* e, int size, CDX12DescriptorHeap* srvHeap) :
		mEngine(e),
		mSize(size)
	{
		mEnable = false;
		mVp = CD3DX12_VIEWPORT(0.f, 0.f, static_cast<float>(mSize), static_cast<float>(mSize));
		mScissorsRect = { 0,0,mSize,mSize };

		mSrvHeap = srvHeap;
		mSrvHandle = srvHeap->Add();

		mDsvHeap = std::make_unique<CDX12DescriptorHeap>(mEngine, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		mRtvHeap = std::make_unique<CDX12DescriptorHeap>(mEngine, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 6);

		mDsvHandle = mDsvHeap->Add();

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
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				&clearValue,
				IID_PPV_ARGS(mResource.GetAddressOf())));

		mResource->SetName(L"AmbientMap");


		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = 1;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.f;

		mEngine->mDevice->CreateShaderResourceView(mResource.Get(), &srvDesc, mSrvHeap->Get(mSrvHandle).mCpu);

		for (int i = 0; i < 6; ++i)
		{
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
			rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Texture2DArray.ArraySize = 1; // Only view one element of the array
			rtvDesc.Texture2DArray.MipSlice = 0;
			rtvDesc.Texture2DArray.PlaneSlice = 0;
			rtvDesc.Texture2DArray.FirstArraySlice = i;

			mEngine->mDevice->CreateRenderTargetView(mResource.Get(), &rtvDesc, mRtvHeap->Get(i).mCpu);
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


		auto cl = CD3DX12_CLEAR_VALUE(dsvDesc.Format, 1.0f, 0);

		ThrowIfFailed(
			mEngine->mDevice->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&dsvDesc,
				D3D12_RESOURCE_STATE_DEPTH_READ,
				&cl,
				IID_PPV_ARGS(mDepthBufferResource.GetAddressOf())));

		mEngine->mDevice->CreateDepthStencilView(mDepthBufferResource.Get(), nullptr, mDsvHeap->Get(mDsvHandle).mCpu);

		NAME_D3D12_OBJECT(mDepthBufferResource);


		for (auto& mConstantBuffer : mConstantBuffers)
		{
			mConstantBuffer = std::make_unique<CDX12ConstantBuffer>(mEngine, mEngine->mSRVDescriptorHeap.get(), sizeof(PerFrameConstants));
		}
	}

	void* CDX12AmbientMap::RenderFromThis(CMatrix4x4* mat)
	{
		if (!mEnable) return nullptr;

		//// Reset all the other command allocators and command lists
		//for (size_t i = 0; i < ARRAYSIZE(mEngine->mAmbientMapCommandLists); ++i)
		//{
		//	auto j = i + mEngine->mCurrentBackBufferIndex * CDX12Engine::mNumFrames;
		//	auto commandAllocator = mEngine->mAmbientMapCommandAllocators[j].Get();
		//	commandAllocator->Reset();
		//	mEngine->mAmbientMapCommandLists[i]->Reset(commandAllocator, nullptr);
		//}

		//auto commandList = mEngine->mAmbientMapCommandLists[0].Get();
		//mEngine->mCurrRecordingCommandList = commandList;

		auto commandList = mEngine->GetCommandList();

		PIXBeginEvent(commandList, 0, L"AmbientMapRendering");

		float mSides[6][3] = {
			// Starting from facing down the +ve Z direction, left handed rotations
			{0.0f, 0.5f, 0.0f},  // +ve X direction (values multiplied by PI)
			{0.0f, -0.5f, 0.0f}, // -ve X direction
			{-0.5f, 0.0f, 0.0f}, // +ve Y direction
			{0.5f, 0.0f, 0.0f},  // -ve Y direction
			{0.0f, 0.0f, 0.0f},  // +ve Z direction
			{0.0f, 1.0f, 0.0f}   // -ve Z direction
		};


		PrepareToRender();

		auto scale = mat->GetScale();
		auto pos = mat->GetPosition();
		auto originalMatrix = *mat;

		for (int i = 0; i < 6; ++i)
		{
			/*
			commandList = mEngine->mAmbientMapCommandLists[i].Get();
			mEngine->mCurrRecordingCommandList = commandList;
			*/

			auto rotation = CVector3(mSides[i]) * PI;

			// To put rotation angles into a matrix we need to build the matrix from scratch to make sure we retain existing scaling and position
			*mat =
				MatrixScaling(scale) *
				MatrixRotationZ(rotation.z) * MatrixRotationX(rotation.x) * MatrixRotationY(rotation.y) *
				MatrixTranslation(pos);

			constexpr FLOAT clearColor[] = { 0.4f,0.6f,0.9f,1.0f };

			CCamera camera(pos, rotation, PI, 1);

			mEngine->SetSkyPSO();

			auto rtv = mRtvHeap->Get(i).mCpu;
			auto dsv = mDsvHeap->Get(mDsvHandle).mCpu;

			commandList->RSSetViewports(1, &mVp);
			commandList->RSSetScissorRects(1, &mScissorsRect);
			commandList->OMSetRenderTargets(1, &rtv, true, &dsv);
			commandList->ClearDepthStencilView(mDsvHeap->Get(mDsvHandle).mCpu, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
			commandList->ClearRenderTargetView(mRtvHeap->Get(i).mCpu, clearColor, 0, nullptr);


			mEngine->GetScene()->RenderSceneFromCamera(&camera);


			// Set camera matrices in the constant buffer and send over to GPU
			//PerFrameConstants perFrameConstants;
			//perFrameConstants.cameraMatrix = camera.WorldMatrix();
			//perFrameConstants.viewMatrix = camera.ViewMatrix();
			//perFrameConstants.projectionMatrix = camera.ProjectionMatrix();
			//perFrameConstants.viewProjectionMatrix = camera.ViewProjectionMatrix();

			//mEngine->mSRVDescriptorHeap->Set();
			//mEngine->SetConstantBuffers();

			//mConstantBuffers[i]->Copy(perFrameConstants);
			//mConstantBuffers[i]->Set(1);

			//auto sky = dynamic_cast<CDX12Sky*>(mEngine->GetObjManager()->mSky);
			//sky->Material()->mAlbedo->Set(6);
			//sky->Render();




			/*
			mEngine->SetPBRPSO();
			mEngine->mSRVDescriptorHeap->Set();
			mConstantBuffers[i]->Set(2);

			mEngine->GetObjManager()->RenderAllObjects();
			*/

			//commandList->Close();

			//ID3D12CommandList* cm[] = { commandList };
			//mEngine->mCommandQueue->ExecuteCommandLists(1, cm);
			//mEngine->mCurrSetPso = nullptr;
		}

		PrepareToShow();

		mEngine->mCurrRecordingCommandList = mEngine->GetCommandList();

		// restore original matrix
		*mat = originalMatrix;

		PIXEndEvent(mEngine->mCurrRecordingCommandList);

		return (void*)mSrvHeap->Get(mSrvHandle).mGpu.ptr;
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
}
