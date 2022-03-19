#pragma once

<<<<<<< HEAD
<<<<<<< Updated upstream
#include "DX12Engine.h"
#include "../Common/CScene.h"
#include "DX12PipelineObject.h"
#include <vector>
=======
=======
#include "DX12Engine.h"
>>>>>>> parent of 5f2c2d1 (Working on IBL - DX12)
#include "../Common/CScene.h"
#include "DX12PipelineObject.h"


#include <vector>

>>>>>>> Stashed changes

namespace DX12
{
	class CDX12DepthStencil;
	class CDX12GameObject;

	class CDX12Scene : public CScene<CDX12Scene>
	{

	public:

		friend class CScene<CDX12Scene>;

		//--------------------------------------------------------------------------------------
		// Constructors
		//--------------------------------------------------------------------------------------

		CDX12Scene() = delete;
		CDX12Scene(const CDX12Scene&) = delete;
		CDX12Scene(const CDX12Scene&&) = delete;
		CDX12Scene& operator=(const CDX12Scene&) = delete;
		CDX12Scene& operator=(const CDX12Scene&&) = delete;

		CDX12Scene(CDX12Engine* engine, std::string  fileName);

		//--------------------------------------------------------------------------------------
		// Scene Render and Update
		//--------------------------------------------------------------------------------------

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
<<<<<<< HEAD
		std::unique_ptr<CDX12DepthStencil> mDepthStencils[3];
		std::unique_ptr<CDX12DescriptorHeap> mDSVDescriptorHeap;
		std::unique_ptr<CDX12AmbientMap> mAmbientMap;
=======
		std::unique_ptr<CDX12DepthStencil> mDepthStencils[CDX12Engine::mNumFrames];
		
		std::unique_ptr<CDX12DescriptorHeap> mDSVDescriptorHeap;
			
>>>>>>> parent of 5f2c2d1 (Working on IBL - DX12)

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

		//--------------------------------------------------------------------------------------
		// Initialization
		//--------------------------------------------------------------------------------------

		void InitFrameDependentStuff();
<<<<<<< Updated upstream
=======

>>>>>>> Stashed changes
	};
}
