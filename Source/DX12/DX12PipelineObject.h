#pragma once
#include "DX12Engine.h"
#include "DX12RootSignature.h"
#include "DX12Shader.h"

namespace DX12
{

	class CDX12PipelineObject
	{
	public:

		CDX12PipelineObject(CDX12Engine* engine, CDX12Shader* vs, CDX12Shader* ps, CDX12RootSignature* rootSignature,
			D3D12_INPUT_ELEMENT_DESC* inputElementDesc, UINT countOfInputElements)
			: mEngine(engine), mRootSignature(rootSignature)
		{
			// Describe and create the graphics pipeline state object (PSO).
			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
			psoDesc.InputLayout = { inputElementDesc, countOfInputElements };
			psoDesc.pRootSignature = mRootSignature->Get();
			psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs->mShaderBlob.Get());
			psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps->mShaderBlob.Get());
			psoDesc.RasterizerState = DirectX::CommonStates::CullCounterClockwise;
			psoDesc.BlendState = DirectX::CommonStates::AlphaBlend;
			psoDesc.DepthStencilState = DirectX::CommonStates::DepthDefault;
			psoDesc.SampleMask = UINT_MAX;
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.NumRenderTargets = 1;
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			psoDesc.DepthStencilState.StencilEnable = FALSE;
			psoDesc.SampleDesc.Count = 1;

			mDesc = psoDesc;

			ThrowIfFailed(engine->mDevice->CreateGraphicsPipelineState(
				&psoDesc, IID_PPV_ARGS(mPipelineState.GetAddressOf())));

			NAME_D3D12_OBJECT(mPipelineState);
		}

		CDX12PipelineObject(CDX12Engine* engine, D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc, CDX12Shader* vs, CDX12Shader* ps, CDX12RootSignature* rootSignature)
			: mEngine(engine), mRootSignature(rootSignature)
		{
			mDesc = psoDesc;

			ThrowIfFailed(engine->mDevice->CreateGraphicsPipelineState(
				&psoDesc, IID_PPV_ARGS(mPipelineState.GetAddressOf())));

			NAME_D3D12_OBJECT(mPipelineState);
		}

		void Set()
		{
			mEngine->mCommandList->SetPipelineState(mPipelineState.Get());
		}

	private:

		CDX12Engine* mEngine;

		ComPtr<ID3D12PipelineState> mPipelineState;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC mDesc;
		CDX12RootSignature* mRootSignature;
	};

	class CDX12PBRPSO
	{
	public:
		CDX12PBRPSO(CDX12Engine* engine, std::vector<D3D12_INPUT_ELEMENT_DESC>& vertexElements, CDX12Shader* vs, CDX12Shader* ps)
			: mEngine(engine)
		{
			mPBRRootSignature = std::make_unique<CDX12PBRRootSignature>(mEngine);

			PSO = std::make_unique<CDX12PipelineObject>(mEngine, vs, ps,
				mPBRRootSignature->mRootSignature.get(), vertexElements.data(),
				static_cast<UINT>(vertexElements.size()));
		}


		void Set(ID3D12GraphicsCommandList* commandList)
		{
			commandList->SetGraphicsRootSignature(mPBRRootSignature->mRootSignature->Get());
			PSO->Set();
		}

		CDX12Engine* mEngine;
		std::unique_ptr<CDX12PipelineObject> PSO;
		std::unique_ptr<CDX12PBRRootSignature> mPBRRootSignature;
	};

}