#include "DX12PointLight.h"

#include "../DX12DescriptorHeap.h"
#include "../DX12PipelineObject.h"
#include "../DX12Texture.h"
#include "../../Common/CGameObjectManager.h"

namespace DX12
{
	void CDX12PointLight::SetShadowMapSize(int size)
	{
		mShadowMapSize = size;

		mDSVDescHeap = nullptr;

		for (auto& map : mShadowMaps)
		{
			map = nullptr;
		}

		InitTextures();
	}

	CDX12PointLight::CDX12PointLight(CDX12Engine* engine,
		const std::string& mesh,
		const std::string& name,
		const std::string& diffuse,
		const CVector3& colour,
		float strength,
		const CVector3& position,
		const CVector3& rotation,
		float scale,
		const int& shadowMapSize) :
		CDX12GameObject(engine, mesh, name, diffuse, position, rotation, scale),
		CPointLight(colour, strength, shadowMapSize),
		mVp(),
		mScissorsRect()
	{
		InitTextures();
	}

	void CDX12PointLight::SetRotation(CVector3 rotation, int node = 0) { CDX12GameObject::SetRotation(rotation, node); }

	void CDX12PointLight::LoadNewMesh(std::string newMesh) { CDX12GameObject::LoadNewMesh(newMesh); }

	void CDX12PointLight::Render(bool basicGeometry) { CDX12GameObject::Render(basicGeometry); }

	void CDX12PointLight::InitTextures()
	{
		mVp = CD3DX12_VIEWPORT(0.f, 0.f, static_cast<float>(mShadowMapSize), static_cast<float>(mShadowMapSize));
		mScissorsRect = { 0,0,mShadowMapSize,mShadowMapSize };

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
		
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.NumDescriptors = 6;

		mDSVDescHeap = std::make_unique<CDX12DescriptorHeap>(mEngine, dsvHeapDesc);

		D3D12_RESOURCE_DESC tDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, mShadowMapSize, mShadowMapSize);

		tDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		
		for (int i = 0; i < 6; ++i)
		{
			mShadowMaps[i] = std::make_unique<CDX12DepthStencil>(mEngine, tDesc, mEngine->mSRVDescriptorHeap.get(), mDSVDescHeap.get());
			mShadowMaps[i]->mResource->SetName(L"ShadowMap");
		}
	}


	void* CDX12PointLight::RenderFromThis()
	{

		// Store original rotation
		const auto originalOrientation = Rotation();

		
		for (int i = 0; i < 6; ++i)
		{
			CVector3 rot = mSides[i];

			CDX12GameObject::SetRotation(rot * PI);

			mShadowMaps[i]->Barrier(D3D12_RESOURCE_STATE_DEPTH_WRITE);

			mEngine->mCommandList->RSSetViewports(0, &mVp);
			mEngine->mCommandList->RSSetScissorRects(1, &mScissorsRect);
			mEngine->mCommandList->OMSetRenderTargets(0, nullptr, false, &mShadowMaps[i]->mDSVHandle.mCpu);
			mEngine->mCommandList->ClearDepthStencilView(mShadowMaps[i]->mDSVHandle.mCpu, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);

			mEngine->mPerFrameConstants.viewMatrix = InverseAffine(WorldMatrix());
			mEngine->mPerFrameConstants.projectionMatrix = MakeProjectionMatrix(1.0f, ToRadians(90.f));
			mEngine->mPerFrameConstants.viewProjectionMatrix = mEngine->mPerFrameConstants.viewMatrix * mEngine->mPerFrameConstants.projectionMatrix;

			mEngine->mPerFrameConstantBuffer->Copy(mEngine->mPerFrameConstants);


			for (const auto& o : mEngine->GetObjManager()->mObjects)
			{
				if (o->IsPbr())
					mEngine->mDepthOnlyTangentPso->Set(mEngine->mCommandList.Get());
				else
					mEngine->mDepthOnlyPso->Set(mEngine->mCommandList.Get());
				o->Render(true);

				mEngine->mCommandList->DrawInstanced(3, 1, 0, 0);
			}

			mShadowMaps[i]->Barrier(D3D12_RESOURCE_STATE_GENERIC_READ);
		}
		
		SetRotation(originalOrientation);

		// unbind the shadow map form render target
		mEngine->mCommandList->OMSetRenderTargets(0, nullptr, false, nullptr);

		return nullptr;
	}


	void* CDX12PointLight::GetSRV()
	{
		return reinterpret_cast<ImTextureID>(mShadowMaps[0]->mHandle.mGpu.ptr);
	}


	CDX12PointLight::~CDX12PointLight()
	{
	}
}
