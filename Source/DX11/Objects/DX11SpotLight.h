#pragma once
#include "DX11Light.h"

namespace DX11
{
	class CDX11SpotLight : public CDX11GameObject, public CSpotLight
	{
	public:
		CDX11SpotLight(CDX11Engine*       engine,
					   const std::string& mesh,
					   const std::string& name,
					   const std::string& diffuse,
			CVector3                      colour   = { 0.0f,0.0f,0.0f },
			float                         strength = 0.0f,
			CVector3                      position = { 0,0,0 },
			CVector3                      rotation = { 0,0,0 },
			float                         scale    = 1);

		CDX11SpotLight(CDX11SpotLight& s);

		void                      Render(bool basicGeometry = false) override;
		ID3D11ShaderResourceView* RenderFromThis();
		void                      SetShadowMapsSize(int value) override;
		void                      SetConeAngle(float value) override;
		~CDX11SpotLight() override;
		void Release() const;


	private:
		void                      InitTextures();
	public:
		void LoadNewMesh(std::string newMesh) override;
	private:
		ID3D11Texture2D*          mShadowMap{};
		ID3D11DepthStencilView*   mShadowMapDepthStencil{};
		ID3D11ShaderResourceView* mShadowMapSRV{};
	};

}
