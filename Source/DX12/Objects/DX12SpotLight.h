#pragma once

#include "DX12GameObject.h"

#include "../../Common/CLight.h"

namespace DX12
{
	class CDX12DepthStencil;

	class CDX12SpotLight final : public CDX12GameObject, public CSpotLight
	{
		public:

			~CDX12SpotLight() override;

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

			void  SetRotation(CVector3 rotation, int node) override;
			void  LoadNewMesh(std::string newMesh) override;
			void  Render(bool basicGeometry) override;
			void* RenderFromThis() override;
			void* GetSRV() override;
			void SetConeAngle(float value) override;
			void SetShadowMapsSize(int value) override;


		private:

			void InitTextures();

		std::unique_ptr<CDX12DescriptorHeap> mDSVDescHeap;
		std::unique_ptr<CDX12DepthStencil> mShadowMap;
		CD3DX12_VIEWPORT mVp;
		RECT mScissorsRect;

	};
}