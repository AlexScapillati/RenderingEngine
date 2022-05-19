#pragma once

#include "DX12Light.h"
#include "../../Common/CLight.h"


namespace DX12
{
	class CDX12DirectionalLight : public CDX12GameObject, public CDirectionalLight
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

			~CDX12DirectionalLight() override = default;
			void  SetShadowMapSize(int s) override;
			void* RenderFromThis() override;

	};
}
