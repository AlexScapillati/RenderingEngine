#pragma once

<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< Updated upstream
=======
>>>>>>> parent of 100d753 (Merge pull request #3 from AlexScapillati/TryingPolymorphism)
=======
>>>>>>> parent of b0bd427 (Up)
=======
<<<<<<< Updated upstream
>>>>>>> parent of 7bb1619 (Merge branch 'main' into TryingPolymorphism)
#include "DX12Engine.h"
#include "../Common/CScene.h"
#include "DX12PipelineObject.h"
=======
<<<<<<< HEAD
>>>>>>> parent of 78525fa (Merge pull request #2 from AlexScapillati/TryingPolymorphism)
#include <vector>
<<<<<<< HEAD
<<<<<<< HEAD
=======
=======
#include "DX12Engine.h"
>>>>>>> parent of 5f2c2d1 (Working on IBL - DX12)
#include "../Common/CScene.h"
#include "DX12PipelineObject.h"

=======
#include "../Common/CScene.h"
>>>>>>> parent of 7bb1619 (Merge branch 'main' into TryingPolymorphism)

#include <vector>
>>>>>>> Stashed changes


>>>>>>> Stashed changes
=======
>>>>>>> parent of 100d753 (Merge pull request #3 from AlexScapillati/TryingPolymorphism)
=======
>>>>>>> parent of b0bd427 (Up)

#include "../Common/CScene.h"

namespace DX12
{
<<<<<<< HEAD
	class CDX12DepthStencil;
	class CDX12GameObject;
=======
class CDX12GameObject;
>>>>>>> parent of 7bb1619 (Merge branch 'main' into TryingPolymorphism)

	class CDX12Scene : public CScene
	{
	public:

		CDX12Scene() = delete;
		CDX12Scene(const CDX12Scene&) = delete;
		CDX12Scene(const CDX12Scene&&) = delete;
		CDX12Scene& operator=(const CDX12Scene&) = delete;
		CDX12Scene& operator=(const CDX12Scene&&) = delete;

		//--------------------------------------------------------------------------------------
		// Constructors
		//--------------------------------------------------------------------------------------

		CDX12Scene(CDX12Engine* engine,
			std::string  fileName);

		//--------------------------------------------------------------------------------------
		// Initialization
		//--------------------------------------------------------------------------------------

		void InitFrameDependentStuff();

		void InitScene();

		//--------------------------------------------------------------------------------------
		// Scene Render and Update
		//--------------------------------------------------------------------------------------

		// Returns the generated scene texture
		void RenderScene(float& frameTime) override;

		ImTextureID GetTextureSRV() override;

		void RenderSceneFromCamera(CCamera* camera) override;
		

		//--------------------------------------------------------------------------------------
		// DirectX12
		//--------------------------------------------------------------------------------------

		void Resize(UINT newX, UINT newY) override;

		~CDX12Scene() override;
		void UpdateScene(float& frameTime) override;
		void Save(std::string fileName) override;
		void PostProcessingPass() override;
		void RenderToDepthMap() override;
		void DisplayPostProcessingEffects() override;

		std::unique_ptr<CDX12RenderTarget> mSceneTexture;
<<<<<<< HEAD
<<<<<<< HEAD
=======
<<<<<<< Updated upstream

		std::unique_ptr<CDX12PBRPSO> mPso;
=======
>>>>>>> parent of 7bb1619 (Merge branch 'main' into TryingPolymorphism)
		std::unique_ptr<CDX12DepthStencil> mDepthStencils[3];
		
		std::unique_ptr<CDX12DescriptorHeap> mDSVDescriptorHeap;
<<<<<<< HEAD
		std::unique_ptr<CDX12AmbientMap> mAmbientMap;
<<<<<<< HEAD
=======
		std::unique_ptr<CDX12DepthStencil> mDepthStencils[CDX12Engine::mNumFrames];
		
		std::unique_ptr<CDX12DescriptorHeap> mDSVDescriptorHeap;
			
>>>>>>> parent of 5f2c2d1 (Working on IBL - DX12)
=======
>>>>>>> parent of 78525fa (Merge pull request #2 from AlexScapillati/TryingPolymorphism)

=======
>>>>>>> Stashed changes
>>>>>>> parent of 7bb1619 (Merge branch 'main' into TryingPolymorphism)

		std::unique_ptr<CDX12AmbientMap> mAmbientMap;
			

	private:
		//--------------------------------------------------------------------------------------
		// Scene Data
		//--------------------------------------------------------------------------------------
		CDX12Engine* mEngine = nullptr;
<<<<<<< HEAD
=======
<<<<<<< Updated upstream
=======

		std::vector<ImTextureID> mShadowMaps;
>>>>>>> parent of 7bb1619 (Merge branch 'main' into TryingPolymorphism)

		std::vector<ImTextureID> mShadowMaps;

<<<<<<< HEAD
=======
		void InitFrameDependentStuff();

>>>>>>> Stashed changes
>>>>>>> parent of 7bb1619 (Merge branch 'main' into TryingPolymorphism)
	};
}