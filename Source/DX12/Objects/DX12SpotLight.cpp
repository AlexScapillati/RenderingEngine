#include "DX12SpotLight.h"

#include "../DX12DescriptorHeap.h"
#include "../DX12PipelineObject.h"
#include "../DX12Texture.h"
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
		CDX12GameObject(engine, mesh, name, diffuse, position, rotation, scale),
		CSpotLight(colour, strength, shadowMapSize, coneAngle)
	{
		InitTextures();
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

		mDSVDescHeap = std::make_unique<CDX12DescriptorHeap>(mEngine,dsvHeapDesc);

		D3D12_RESOURCE_DESC tDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, mShadowMapSize, mShadowMapSize);

		tDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		mShadowMap = std::make_unique<CDX12DepthStencil>(mEngine, tDesc, mEngine->mSRVDescriptorHeap.get(), mDSVDescHeap.get());
	}

	void CDX12SpotLight::SetRotation(CVector3 rotation, int node) { CGameObject::SetRotation(rotation, node); }
	void CDX12SpotLight::LoadNewMesh(std::string newMesh) { CDX12GameObject::LoadNewMesh(newMesh); }
	void CDX12SpotLight::Render(bool basicGeometry) { CDX12GameObject::Render(basicGeometry); }

	void* CDX12SpotLight::RenderFromThis()
	{

		auto commandList = mEngine->mCommandList.Get();

		mShadowMap->Barrier(D3D12_RESOURCE_STATE_DEPTH_WRITE);

		commandList->RSSetViewports(1, &mVp);
		commandList->RSSetScissorRects(1, &mScissorsRect);

		commandList->OMSetRenderTargets(0, nullptr, false,&mShadowMap->mDSVHandle.mCpu);
		commandList->ClearDepthStencilView(mShadowMap->mDSVHandle.mCpu,D3D12_CLEAR_FLAG_DEPTH,1.f,0,0,nullptr);
			 
		for(const auto& o : mEngine->GetObjManager()->mObjects)
		{
			if (o->IsPbr())
				mEngine->mDepthOnlyTangentPso->Set(commandList);
			else
				mEngine->mDepthOnlyPso->Set(commandList);
			o->Render(true);
		}

		commandList->DrawInstanced(3, 1, 0, 0);

		mShadowMap->Barrier(D3D12_RESOURCE_STATE_GENERIC_READ);

		return &mShadowMap->mHandle.mGpu;
	}

	void* CDX12SpotLight::GetSRV()
	{
		return reinterpret_cast<ImTextureID>(mShadowMap->mHandle.mGpu.ptr);
	}

	CDX12SpotLight::~CDX12SpotLight()
	{
	}
}
