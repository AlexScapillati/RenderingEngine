#pragma once

#include <d3d11.h>

#include "DX11Light.h"

class CVector3;


namespace DX11
{
	class CDX11Engine;

	class CDX11DirLight : public CDX11Light, public CDirectionalLight
	{
		public:

			CDX11DirLight(CDX11Engine* engine, 
					std::string    mesh, 
					std::string    name,
					std::string&   diffuse, 
					CVector3       colour   = { 0.0f, 0.0f, 0.0f }, 
					float          strength = 0.0f,
					CVector3       position = { 0, 0, 0 }, 
					CVector3       rotation = { 0, 0, 0 }, 
					float          scale    = 1);

			CDX11DirLight(CDX11DirLight& l);

			ID3D11ShaderResourceView* RenderFromThis();

			void                      SetShadowMapSize(int s) override;

			~CDX11DirLight() override;

			void Release() const;

			void InitTextures();

		private:


			ID3D11Texture2D*          mShadowMap;
			ID3D11DepthStencilView*   mShadowMapDepthStencil;
			ID3D11ShaderResourceView* mShadowMapSRV;
	};
	
}
