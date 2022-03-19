#pragma once

#include "DX12Common.h"

#include "DX12RootSignature.h"

namespace DX12
{
	class CDX12Engine;

	class CDX12PBRPSO
	{
	public:
		CDX12PBRPSO(CDX12Engine* engine);

		void Set(ID3D12GraphicsCommandList* commandList);

		CDX12Engine* mEngine;
		std::unique_ptr<CDX12PBRRootSignature> mPBRRootSignature;
		ComPtr<ID3D12PipelineState> mPipelineState;
	};

	class CDX12SkyPSO
	{
	public:
		CDX12SkyPSO(CDX12Engine* engine);

		void Set(ID3D12GraphicsCommandList* commandList);

		CDX12Engine* mEngine;
		std::unique_ptr<CDX12SkyRootSignature> mRootSignature;
		ComPtr<ID3D12PipelineState> mPipelineState;
	};

	class CDX12DepthOnlyPSO
	{
		public:

		CDX12DepthOnlyPSO(CDX12Engine* engine, bool requireTangents);

		void Set(ID3D12GraphicsCommandList* commandList);

		CDX12Engine* mEngine;
		std::unique_ptr<CDX12DepthOnlyRootSignature> mRootSignature;
		ComPtr<ID3D12PipelineState> mPipelineState;
	};

}
