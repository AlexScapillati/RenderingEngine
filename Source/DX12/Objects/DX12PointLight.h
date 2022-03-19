#pragma once

#include "DX12GameObject.h"
#include "../../Common/CLight.h"

namespace DX12
{
	class CDX12DepthStencil;

	class CDX12PointLight final : public CDX12GameObject, public CPointLight
	{
		public:
			CDX12PointLight(CDX12Engine*       engine,
							const std::string& mesh,
							const std::string& name,
							const std::string& diffuse,
							const CVector3&    colour,
							float              strength,
							const CVector3&    position,
							const CVector3&    rotation,
							float              scale,
							const int&         shadowMapSize = 2048);

			void  SetRotation(CVector3 rotation, int node) override;
			void  LoadNewMesh(std::string newMesh) override;
			void  Render(bool basicGeometry) override;
			void  InitTextures();
			void* RenderFromThis() override;
			void* GetSRV() override;
			void  SetShadowMapSize(int size) override;


		~CDX12PointLight() override;

		std::unique_ptr<CDX12DescriptorHeap> mDSVDescHeap;
		std::unique_ptr<CDX12DepthStencil> mShadowMaps[6];
		CD3DX12_VIEWPORT mVp;
		RECT mScissorsRect;

	};
}