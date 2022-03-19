//--------------------------------------------------------------------------------------
// Scene geometry and layout preparation
// Scene rendering & update
//--------------------------------------------------------------------------------------

#pragma once

#include "../Common/CScene.h"

#include "..\Math/CVector3.h"
#include "GraphicsHelpers.h" // Helper functions to unclutter the code here

#include <array>
#include <list>
#include <stdexcept>
#include <utility>
#include <vector>
#include <wrl.h>
#include "../Math/CVector2.h"

class CMatrix4x4;
class CWindow;
class CCamera;

namespace DX11
{

	class CDX11Scene : public CScene
	{
	public:


		void InitTextures();

		CDX11Scene(CDX11Engine* engine, std::string fileName);

		void RenderSceneFromCamera(CCamera* camera) override;

		//--------------------------------------------------------------------------------------
		// Scene Render and Update
		//--------------------------------------------------------------------------------------

		// Returns the generated scene texture
		void RenderScene(float& frameTime) override;

		void UpdateScene(float& frameTime) override;

		//--------------------------------------------------------------------------------------
		// Public Functions
		//--------------------------------------------------------------------------------------

		void Save(std::string fileName = "") override;
		void Resize(UINT newX, UINT newY) override;
		void PostProcessingPass() override;
		void RenderToDepthMap() override;
		void DisplayPostProcessingEffects() override; // TODO: Remove


		ImTextureID GetTextureSRV() override
		{
			return mSceneSRV.Get();
		}

		std::vector<ID3D11ShaderResourceView*> mShadowsMaps;

	private:

		//--------------------------------------------------------------------------------------
		// Scene Data
		//--------------------------------------------------------------------------------------

		CDX11Engine* mEngine = nullptr;


		ComPtr<ID3D11Texture2D> mTextrue;
		ComPtr<ID3D11ShaderResourceView> mSceneSRV;
		ComPtr<ID3D11RenderTargetView> mSceneRTV;

		ComPtr<ID3D11Texture2D> mDepthStencil;
		ComPtr<ID3D11ShaderResourceView> mDepthStencilSRV;
		ComPtr<ID3D11DepthStencilView> mDepthStencilRTV;

		ComPtr<ID3D11Texture2D> mFinalTextrue;
		ComPtr<ID3D11ShaderResourceView> mFinalTextureSRV;
		ComPtr<ID3D11RenderTargetView> mFinalRTV;

		ComPtr<ID3D11Texture2D> mFinalDepthStencil;
		ComPtr<ID3D11ShaderResourceView> mFinalDepthStencilSRV;
		ComPtr<ID3D11DepthStencilView> mFinalDepthStencilRTV;

		//****************************
		// Post processing textures

		// Additional textures used for specific post-processes
		ComPtr<ID3D11Resource> mNoiseMap = nullptr;
		ComPtr<ID3D11ShaderResourceView> mNoiseMapSRV = nullptr;

		ComPtr<ID3D11Resource> mBurnMap = nullptr;
		ComPtr<ID3D11ShaderResourceView> mBurnMapSRV = nullptr;

		ComPtr<ID3D11Resource> mDistortMap = nullptr;
		ComPtr<ID3D11ShaderResourceView> mDistortMapSRV = nullptr;

		ComPtr<ID3D11Texture2D> mLuminanceMap = nullptr;
		ComPtr<ID3D11ShaderResourceView> mLuminanceMapSRV = nullptr;
		ComPtr<ID3D11RenderTargetView> mLuminanceRTV = nullptr;

		ComPtr<ID3D11Resource> mRandomMap = nullptr;
		ComPtr<ID3D11ShaderResourceView> mRandomMapSRV = nullptr;

		bool mSsaoBlur = false;

		ComPtr<ID3D11Texture2D > mSsaoMap = nullptr;
		ComPtr<ID3D11ShaderResourceView > mSsaoMapSRV = nullptr;
		ComPtr<ID3D11RenderTargetView > mSsaoMapRTV = nullptr;

		//****************************

		//--------------------------------------------------------------------------------------
		// PostProcess Functions
		//--------------------------------------------------------------------------------------

		// Select the appropriate shader plus any additional textures required for a given post-process
		// Helper function shared by full-screen, area and polygon post-processing functions below
		void SelectPostProcessShaderAndTextures(PostProcess postProcess);

		// Perform a full-screen post process from "scene texture" to back buffer
		void FullScreenPostProcess(PostProcess postProcess);

		// Perform an area post process from "scene texture" to back buffer at a given point in the world, with a given size (world units)
		void AreaPostProcess(PostProcess postProcess, CVector3 worldPoint, CVector2 areaSize);

		// Perform an post process from "scene texture" to back buffer within the given four-point polygon and a world matrix to position/rotate/scale the polygon
		void PolygonPostProcess(PostProcess postProcess, const std::array<CVector3, 4>& points, const CMatrix4x4& worldMatrix);

		//to remove
		void LoadPostProcessingImages();

		void ReleasePostProcessingShaders();
	};
}