#pragma once

#include "DX12Light.h"

namespace DX12
{
	class CDX12DepthStencil;

	class CDX12PointLight : public CDX12Light
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
			void* RenderFromThis() ;
			void* GetSRV() ;
			void  SetShadowMapSize(int size) ;

int GetShadowMapSize() const { return mShadowMapSize; }

		float mSides[6][3] = {          // Starting from facing down the +ve Z direction, left handed rotations
				{ 0.0f,	 0.5f,	0.0f},  // +ve X direction (values multiplied by PI)
				{ 0.0f, -0.5f,	0.0f},  // -ve X direction
				{-0.5f,	 0.0f,	0.0f},  // +ve Y direction
				{ 0.5f,	 0.0f,	0.0f},  // -ve Y direction
				{ 0.0f,	 0.0f,	0.0f},  // +ve Z direction
				{ 0.0f,	 1.0f,  0.0f}   // -ve Z direction
		};

	protected:

		int mShadowMapSize;


		virtual ~CDX12PointLight() override = default;

		std::unique_ptr<CDX12DescriptorHeap> mDSVDescHeap;
		std::unique_ptr<CDX12DepthStencil> mShadowMaps[6];
		CD3DX12_VIEWPORT mVp;
		RECT mScissorsRect;

	};
}