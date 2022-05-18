#pragma once

#include "DX11Light.h"

class CVector3;

namespace DX11
{
	class CDX11Engine;

	class CDX11DirLight final : public CDX11GameObject, public CDirectionalLight
	{
		public:
			CDX11DirLight(CDX11Engine*       engine,
						  const std::string& mesh,
						  const std::string& name,
						  const std::string& diffuse,
						  CVector3           colour   = { 0.0f,0.0f,0.0f },
						  float              strength = 0.0f,
						  CVector3           position = { 0,0,0 },
						  CVector3           rotation = { 0,0,0 },
						  float              scale    = 1);

			void  InitTextures();
			void  Render(bool basicGeometry) override;
			void* RenderFromThis() override;
			void  SetShadowMapSize(int s) override;

			void Release() const;
			~CDX11DirLight() override;



		private:


			ID3D11Texture2D*          mShadowMap;
			ID3D11DepthStencilView*   mShadowMapDepthStencil;
			ID3D11ShaderResourceView* mShadowMapSRV;
	};
	
}
