#include "CDX12Sky.h"

#include "../../Common/CScene.h"
#include "../DX12PipelineObject.h"

namespace DX12
{
	void CDX12Sky::Render(bool basicGeometry)
	{
		//if the model is not enable do not render it
		if (!mEnabled) return;

		SetPosition(mEngine->GetScene()->GetCamera()->Position());

		mMesh->mModelConstants.objectColour = { 1.f,1.f,1.f };

		// Set the pipeline state object
		mEngine->mSkyPso->Set(mEngine->mCommandList.Get());

		mMaterial->RenderMaterial();

		mEngine->SetConstantBuffers();

		// Render the mesh
		mMesh->Render(mWorldMatrices);
	}

	void CDX12Sky::LoadNewMesh(std::string newMesh) { CDX12GameObject::LoadNewMesh(newMesh); }
}
