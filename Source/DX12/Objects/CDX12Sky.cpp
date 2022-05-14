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
		mMaterial->RenderMaterial();

		auto& cb = mMesh->ModelConstants();
		cb.hasAoMap = mMaterial->mAo ? 1 : 0;
		cb.hasNormalMap = mMaterial->mNormal ? 1 : 0;
		cb.hasMetallnessMap = mMaterial->mMetalness ? 1 : 0;
		cb.hasRoughnessMap = mMaterial->mRoughness ? 1 : 0;
		cb.hasDisplacementMap = mMaterial->mDisplacement ? 1 : 0;
		cb.roughness = mRoughness;
		cb.metalness = mMetalness;
		cb.parallaxDepth = mParallaxDepth;
		cb.useCustomValues = 0;

		// Render the mesh
		mMesh->Render(mWorldMatrices);
	}
	
}
