#pragma once

<<<<<<< Updated upstream
#include "DX12Engine.h"
#include "../Common/CScene.h"
#include "DX12PipelineObject.h"
=======
#include "../Common/CScene.h"

#include <vector>
>>>>>>> Stashed changes


namespace DX12
{
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

		void InitScene();

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
<<<<<<< Updated upstream

		std::unique_ptr<CDX12PBRPSO> mPso;
=======
		std::unique_ptr<CDX12DepthStencil> mDepthStencils[3];
		std::unique_ptr<CDX12DescriptorHeap> mDSVDescriptorHeap;
		std::unique_ptr<CDX12AmbientMap> mAmbientMap;
>>>>>>> Stashed changes

		//--------------------------------------------------------------------------------------
		// Scene Data
		//--------------------------------------------------------------------------------------

	private:

		CDX12Engine* mEngine = nullptr;
<<<<<<< Updated upstream
=======

		std::vector<ImTextureID> mShadowMaps;

		//--------------------------------------------------------------------------------------
		// Initialization
		//--------------------------------------------------------------------------------------

		void InitFrameDependentStuff();

>>>>>>> Stashed changes
	};
}
