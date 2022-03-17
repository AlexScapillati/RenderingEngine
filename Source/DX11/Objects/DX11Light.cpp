#include "DX11Light.h"

#include "../DX11Material.h"

namespace DX11
{
	CDX11Light::CDX11Light(CDX11Engine* engine,
							const std::string& mesh,
							const std::string& name,
							const std::string& diffuse,
							const CVector3& colour,
							const float& strength,
							const CVector3& position ,
							const CVector3& rotation,
							const float& scale)
	:
	CLight(colour, strength),
	CDX11GameObject(engine, mesh, name, diffuse, position, rotation, scale)
	{
		try
		{
			mMaterial->SetVertexShader(mEngine->mBasicTransformVertexShader.Get());
			mMaterial->SetPixelShader(mEngine->mTintedTexturePixelShader.Get());
		}
		catch (const std::exception& e)
		{
			throw std::exception(e.what());
		}
	}

	void CDX11Light::Render(bool basicGeometry)
	{
		if (basicGeometry)
		{
			//render object
			CDX11GameObject::Render();
		}
		else
		{
			// Get Previous states
			ID3D11RasterizerState* prevRS = nullptr;
			ID3D11BlendState* prevBS = nullptr;
			ID3D11DepthStencilState* prevDSS = nullptr;
			UINT                     prevStencilRef = 0;
			UINT                     prevSampleMask = 0xffffff;
			FLOAT* prevBlendFactor = nullptr;

			mEngine->GetContext()->RSGetState(&prevRS);
			mEngine->GetContext()->OMGetBlendState(&prevBS, prevBlendFactor, &prevSampleMask);
			mEngine->GetContext()->OMGetDepthStencilState(&prevDSS, &prevStencilRef);

			gPerModelConstants.objectColour = mColour;

			// States - additive blending, read-only depth buffer and no culling (standard set-up for blending)
			mEngine->GetContext()->OMSetBlendState(mEngine->mAdditiveBlendingState.Get(), nullptr, 0xffffff);
			mEngine->GetContext()->OMSetDepthStencilState(mEngine->mDepthReadOnlyState.Get(), 0);
			mEngine->GetContext()->RSSetState(mEngine->mCullNoneState.Get());

			//render object
			CDX11GameObject::Render(false);

			// Set back the prev states
			mEngine->GetContext()->RSSetState(prevRS);
			mEngine->GetContext()->OMSetBlendState(prevBS, prevBlendFactor, prevSampleMask);
			mEngine->GetContext()->OMSetDepthStencilState(prevDSS, prevStencilRef);

			if (prevRS) prevRS->Release();
			if (prevBS) prevBS->Release();
			if (prevDSS) prevDSS->Release();

			delete[] prevBlendFactor;
		}
	}

	void CDX11Light::LoadNewMesh(std::string newMesh) { CDX11GameObject::LoadNewMesh(newMesh); }
	CDX11Light::~CDX11Light()
	{
	}
}
