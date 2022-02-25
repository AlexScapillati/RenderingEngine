#pragma once

#include "DX12Engine.h"
#include "../Common/CScene.h"
#include "DX12PipelineObject.h"


namespace DX12
{
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

		std::unique_ptr<CDX12PBRPSO> mPso;

	private:
		//--------------------------------------------------------------------------------------
		// Scene Data
		//--------------------------------------------------------------------------------------
		CDX12Engine* mEngine = nullptr;
	};
}