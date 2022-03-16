#pragma once

#include "DX11Light.h"


namespace DX11
{
	class CDX11PointLight : public CDX11Light, public CPointLight
	{
	public:

		CDX11PointLight(CDX11Engine* engine,
			const std::string& mesh,
			const std::string& name,
			const std::string& diffuse, 
			const CVector3& colour,
			const float& strength, 
			CVector3 position = { 0.0f,0.0f,0.0f },
			CVector3 rotation = { 0.0f,0.0f,0.0f },
			float scale = 1.0f);

		CDX11PointLight(CDX11PointLight& p);

		void Render(bool basicGeometry = false) override;
		void SetShadowMapSize(int size) override;

		ID3D11ShaderResourceView** RenderFromThis();

		~CDX11PointLight() override;

		void Release();


	private:


		ID3D11Texture2D* mShadowMap[6];
		ID3D11DepthStencilView* mShadowMapDepthStencils[6];
		ID3D11ShaderResourceView* mShadowMapSRV[6];

		void InitTextures();
	public:
		void LoadNewMesh(std::string newMesh) override;
	};
}