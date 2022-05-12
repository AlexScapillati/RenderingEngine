#pragma once

<<<<<<< HEAD
=======
<<<<<<< Updated upstream
#include "DX12Engine.h"
#include "../Common/CScene.h"
#include "DX12PipelineObject.h"
#include <vector>
=======
#include "../Common/CScene.h"

>>>>>>> parent of a9c1de14 (revert commit)
#include <vector>

#include "../Common/CScene.h"

namespace DX12
{
	class CDX12DescriptorHeap;
	class CDX12RenderTarget;
	class CDX12Engine;
	class CDX12AmbientMap;
	class CDX12DepthStencil;
	class CDX12GameObject;

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

<<<<<<< HEAD
		CDX12Scene(CDX12Engine* engine,
			std::string  fileName);
=======
		// Returns the generated scene texture
		void RenderSceneImpl(float& frameTime) ;


		void RenderSceneFromCameraImpl(CCamera* camera) ;
		

		//--------------------------------------------------------------------------------------
		// DirectX12
		//--------------------------------------------------------------------------------------

		void ResizeImpl(UINT newX, UINT newY);
		void PostProcessingPassImpl();
		void RenderToDepthMapImpl();
		void DisplayPostProcessingEffectsImpl();
		ImTextureID GetTextureSRVImpl() ;

		std::unique_ptr<CDX12RenderTarget> mSceneTexture;
		std::unique_ptr<CDX12DepthStencil> mDepthStencils[3];
		std::unique_ptr<CDX12DescriptorHeap> mDSVDescriptorHeap;
		std::unique_ptr<CDX12AmbientMap> mAmbientMap;

<<<<<<< Updated upstream

		//--------------------------------------------------------------------------------------
		// Scene Data
		//--------------------------------------------------------------------------------------
=======
		//--------------------------------------------------------------------------------------
		// Scene Data
		//--------------------------------------------------------------------------------------

	private:

		CDX12Engine* mEngine = nullptr;
>>>>>>> Stashed changes

	private:

		CDX12Engine* mEngine = nullptr;
		std::vector<ImTextureID> mShadowMaps;
>>>>>>> parent of a9c1de14 (revert commit)

		//--------------------------------------------------------------------------------------
		// Initialization
		//--------------------------------------------------------------------------------------

		void InitFrameDependentStuff();

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
		std::unique_ptr<CDX12DepthStencil> mDepthStencils[3];

		std::unique_ptr<CDX12DescriptorHeap> mDSVDescriptorHeap;
		std::unique_ptr<CDX12AmbientMap>     mAmbientMap;


	private:
		//--------------------------------------------------------------------------------------
		// Scene Data
		//--------------------------------------------------------------------------------------
		CDX12Engine* mEngine = nullptr;

		std::vector<ImTextureID> mShadowMaps;

	};
}