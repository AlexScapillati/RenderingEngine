#include "DX12DirectionalLight.h"

namespace DX12
{
	CDX12DirectionalLight::CDX12DirectionalLight(CDX12Engine*       engine,
												 const std::string& mesh,
												 const std::string& name,
												 const std::string& diffuse,
												 const CVector3&    colour,
												 const float&       strength,
												 const CVector3&    position,
												 const CVector3&    rotation,
												 const float&       scale,
												 const int&         shadowMapSize,
												 const float&       width,
												 const float&       height,
												 const float&       nearClip,
												 const float&       farClip)
		:
		CDX12Light(engine, mesh, name, diffuse, colour, strength, position, rotation, scale),
		mShadowMapSize(shadowMapSize), mWidth(width), mHeight(height), mNearClip(nearClip), mFarClip(farClip)
	{
	}

	void CDX12DirectionalLight::SetShadowMapSize(int s)
	{
	}

	void* CDX12DirectionalLight::RenderFromThis()
	{
		return nullptr;
	}
}
