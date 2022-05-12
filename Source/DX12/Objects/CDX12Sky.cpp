#include "CDX12Sky.h"

#include "../DX12Engine.h"
#include "../DX12Scene.h"
#include "../../Common/Camera.h"

namespace DX12
{
	void CDX12Sky::Render(bool basicGeometry)
	{
		//if the model is not enable do not render it
		if (!mEnabled) return;

		SetPosition(mEngine->GetScene()->GetCamera()->Position());

		mMesh->mModelConstants.objectColour = { 1.f,1.f,1.f };

		mEngine->SetSkyPSO();

		mMaterial->RenderMaterial();

		mEngine->SetConstantBuffers();

		// Render the mesh
		mMesh->Render(mWorldMatrices);
	}
	
}
