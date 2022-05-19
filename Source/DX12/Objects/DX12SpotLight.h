#pragma once

#include "DX12GameObject.h"

#include "../../Common/CLight.h"

namespace DX12
{
	class CDX12DepthStencil;

	class CDX12SpotLight : public CDX12GameObject, public CSpotLight
	{
		public:

			virtual ~CDX12SpotLight() override;

			CDX12SpotLight(CDX12Engine*       engine,
						   const std::string& mesh,
						   const std::string& name,
						   const std::string& diffuse,
						   const CVector3&    colour,
						   float              strength,
						   const CVector3&    position,
						   const CVector3&    rotation,
						   float              scale,
						   const int&         shadowMapSize = 2048,
						   const float&       coneAngle     = 90.f);
			
			void* RenderFromThis() override;
			void* GetSRV() override;
			void SetConeAngle(float value) override;
			void SetShadowMapsSize(int value) override;

			uint32_t mSrvHandle;

	private:
			void InitTextures();

			std::unique_ptr<CDX12DescriptorHeap> mDSVDescHeap;
			CDX12DescriptorHeap* mSrvHeap;

			ComPtr<ID3D12Resource> mShadowMapResource;

			uint32_t mDsvHandle;


			ComPtr<ID3D12GraphicsCommandList4> mCommandList;
			ComPtr<ID3D12CommandAllocator> mCommandAllocators[3];

			CD3DX12_VIEWPORT mVp;
			RECT mScissorsRect;

	};
}