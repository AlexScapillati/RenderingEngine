#pragma once

#include "DX12GameObject.h"
#include "DX12Light.h"

namespace DX12
{
	class CDX12DepthStencil;

	class CDX12SpotLight : public CDX12Light
	{
	public:

		virtual ~CDX12SpotLight() override;

		CDX12SpotLight(CDX12Engine* engine,
			const std::string& mesh,
			const std::string& name,
			const std::string& diffuse,
			const CVector3& colour,
			float              strength,
			const CVector3& position,
			const CVector3& rotation,
			float              scale,
			const int& shadowMapSize = 2048,
			const float& coneAngle = 90.f);

		void* RenderFromThis();
		void* GetSRV();
		void SetConeAngle(float value);
		void SetShadowMapsSize(int value);

		int& GetShadowMapSize() { return mShadowMapSize; }
		float& GetConeAngle() { return mConeAngle; }


	private:

		int   mShadowMapSize = 0;
		float mConeAngle = 0;

		void InitTextures();

		std::unique_ptr<CDX12DescriptorHeap> mDSVDescHeap;

		ComPtr<ID3D12Resource> mShadowMapResource;

		SHandle* mSrvHandle;
		SHandle* mDsvHandle;

		ComPtr<ID3D12GraphicsCommandList4> mCommandList;
		ComPtr<ID3D12CommandAllocator> mCommandAllocators[3];

		CD3DX12_VIEWPORT mVp;
		RECT mScissorsRect;

	};
}