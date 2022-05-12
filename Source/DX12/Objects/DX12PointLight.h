#pragma once

#include "DX12GameObject.h"
#include "../../Common/CLight.h"

namespace DX12
{
	class CDX12DepthStencil;

	class CDX12PointLight : virtual public CDX12GameObject, virtual public CPointLight
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

		
			void  InitTextures();
			void* RenderFromThis() override;
			void* GetSRV() override;
			void  SetShadowMapSize(int size) override;


		virtual ~CDX12PointLight() override = default;

		std::unique_ptr<CDX12DescriptorHeap> mDSVDescHeap;
		std::unique_ptr<CDX12DepthStencil> mShadowMaps[6];
		CD3DX12_VIEWPORT mVp;
		RECT mScissorsRect;

	};
}