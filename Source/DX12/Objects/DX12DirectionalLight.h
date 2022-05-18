#pragma once

#include "DX12Light.h"

namespace DX12
{
	class CDX12DirectionalLight : public CDX12Light
	{
		public:
			CDX12DirectionalLight(CDX12Engine*       engine,
								  const std::string& mesh,
								  const std::string& name,
								  const std::string& diffuse,
								  const CVector3&    colour,
								  const float&       strength,
								  const CVector3&    position,
								  const CVector3&    rotation,
								  const float&       scale,
								  const int&         shadowMapSize = 2048,
								  const float&       width         = 1000,
								  const float&       height        = 1000,
								  const float&       nearClip      = 0.001f,
								  const float&       farClip       = 1000);
			
			virtual ~CDX12DirectionalLight() = default;
			void    SetShadowMapSize(int s);
			void*   RenderFromThis();


			auto GetNearClip() const { return mNearClip; }
			auto GetFarClip() const { return mFarClip; }
			auto SetNearClip(float n) { mNearClip = n; }
			auto SetFarClip(float n) { mFarClip = n; }
			auto GetWidth() const { return mWidth; }
			auto GetHeight() const { return mHeight; }
			auto SetWidth(float n) { mWidth = n; }
			auto SetHeight(float n) { mHeight = n; }
			auto GetShadowMapSize() const { return mShadowMapSize; }

	protected:
		int   mShadowMapSize;
		float mWidth;
		float mHeight;
		float mNearClip;
		float mFarClip;
	};
}
