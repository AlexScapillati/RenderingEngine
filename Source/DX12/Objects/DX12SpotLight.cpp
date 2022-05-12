#include "DX12SpotLight.h"

#include "../DX12Engine.h"

#include "../DX12DescriptorHeap.h"
#include "../DX12PipelineObject.h"
#include "../../Common/CGameObjectManager.h"

namespace DX12
{

	CDX12SpotLight::CDX12SpotLight(CDX12Engine* engine,
		const std::string& mesh,
		const std::string& name,
		const std::string& diffuse,
		const CVector3& colour,
		float strength,
		const CVector3& position,
		const CVector3& rotation,
		float scale,
		const int& shadowMapSize,
		const float& coneAngle) :
		CDX12Light(engine, mesh, name, diffuse,colour,strength, position, rotation, scale),
	mShadowMapSize(shadowMapSize),
	mConeAngle(coneAngle)
	{
		InitTextures();

		for (auto& commandAllocator : mCommandAllocators)
		{
			ThrowIfFailed(mEngine->mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,IID_PPV_ARGS(commandAllocator.GetAddressOf())))
		}

		ThrowIfFailed(mEngine->mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocators[0].Get(), nullptr ,IID_PPV_ARGS(mCommandList.GetAddressOf())));
		mCommandList->Close();
	}

	void CDX12SpotLight::SetConeAngle(float value) { mConeAngle = value; }
	void CDX12SpotLight::SetShadowMapsSize(int value) { mShadowMapSize = value; }

	void CDX12SpotLight::InitTextures()
	{

		mVp           = CD3DX12_VIEWPORT(0.f, 0.f, static_cast<float>(mShadowMapSize), static_cast<float>(mShadowMapSize));
		mScissorsRect = {0,0,mShadowMapSize,mShadowMapSize};

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc {};

		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.NumDescriptors = 1;

		// Create Descriptor heap that will hold the texture
		mDSVDescHeap = std::make_unique<CDX12DescriptorHeap>(mEngine,dsvHeapDesc);

		mDsvHandle = mDSVDescHeap->Get(mDSVDescHeap->Add());
		mSrvHandle = mEngine->mSRVDescriptorHeap->Get(mEngine->mSRVDescriptorHeap->Add());

		auto heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		// Create the texture resource

		D3D12_RESOURCE_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Alignment = 0;
		texDesc.Width = mShadowMapSize;
		texDesc.Height = mShadowMapSize;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = DXGI_FORMAT_D32_FLOAT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = DXGI_FORMAT_D32_FLOAT;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;

		ThrowIfFailed(mEngine->mDevice->CreateCommittedResource(
			&heap,
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			&optClear,
			IID_PPV_ARGS(mShadowMapResource.GetAddressOf())));
		

		// Create SRV to resource so we can sample the shadow map in a shader program.
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		srvDesc.Texture2D.PlaneSlice = 0;
		mEngine->mDevice->CreateShaderResourceView(mShadowMapResource.Get(), &srvDesc, mSrvHandle->mCpu);


		// Create DSV to resource so we can render to the shadow map.
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.Texture2D.MipSlice = 0;
		mEngine->mDevice->CreateDepthStencilView(mShadowMapResource.Get(), &dsvDesc, mDsvHandle->mCpu);
	}

	void* CDX12SpotLight::RenderFromThis()
	{
		mEngine->mCurrRecordingCommandList = mCommandList.Get();
		auto commandList = mEngine->mCurrRecordingCommandList;

		mCommandAllocators[mEngine->mCurrentBackBufferIndex]->Reset();
		mCommandList->Reset(mCommandAllocators[mEngine->mCurrentBackBufferIndex].Get(), nullptr);

		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMapResource.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		commandList->ResourceBarrier(1,&barrier);

		mEngine->mCurrSetPso = nullptr;
		mEngine->SetDepthOnlyPSO();
		mEngine->mSRVDescriptorHeap->Set();

		commandList->RSSetViewports(1, &mVp);
		commandList->RSSetScissorRects(1, &mScissorsRect);
		commandList->ClearDepthStencilView(mDsvHandle->mCpu,D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,1.f,0,0,nullptr);
		commandList->OMSetRenderTargets(0, nullptr, false,&mDsvHandle->mCpu);

		auto i = mEngine->mCurrentBackBufferIndex;

		mEngine->mPerFrameConstants[i].viewMatrix = InverseAffine(WorldMatrix());
		mEngine->mPerFrameConstants[i].projectionMatrix = MakeProjectionMatrix(1.0f, ToRadians(mConeAngle));
		mEngine->mPerFrameConstants[i].viewProjectionMatrix = mEngine->mPerFrameConstants[i].viewMatrix * mEngine->mPerFrameConstants[i].projectionMatrix;

		mEngine->mPerFrameConstantBuffer[i]->Copy(mEngine->mPerFrameConstants);

		for(const auto& o : mEngine->GetObjManager()->mObjects)
		{
			mEngine->mPerFrameConstantBuffer[i]->Set(1);
			o->Render(true);
		}

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMapResource.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandList->ResourceBarrier(1, &barrier);

		// Execute the commandList
		ThrowIfFailed(commandList->Close());

		ID3D12CommandList* const commandLists[] = { commandList };
		mEngine->mCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

		mEngine->mCurrSetPso = nullptr;
		mEngine->mCurrRecordingCommandList = mEngine->GetCommandList();

		mEngine->WaitForGpu();

		return &mSrvHandle->mGpu;
	}

	void* CDX12SpotLight::GetSRV()
	{
		return reinterpret_cast<ImTextureID>(mSrvHandle->mGpu.ptr);
	}

	CDX12SpotLight::~CDX12SpotLight()
	{
	}
}
