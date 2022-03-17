#include "DX12PipelineObject.h"
#include "DX12RootSignature.h"
#include "DX12Shader.h"

namespace DX12
{

	CDX12PBRPSO::CDX12PBRPSO(CDX12Engine* engine) : mEngine(engine)
	{
		try
		{
			mPBRRootSignature = std::make_unique<CDX12PBRRootSignature>(mEngine);

			auto vs = engine->vs.get();
			auto ps = engine->ps.get();

			// Check for presence of position and normal data. Tangents and UVs are optional.
			std::vector<D3D12_INPUT_ELEMENT_DESC> vertexElements;
			vertexElements.push_back({ "position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			vertexElements.push_back({ "normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			vertexElements.push_back({ "tangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			vertexElements.push_back({ "uv", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

			// Describe and create the graphics pipeline state object (PSO).
			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
			psoDesc.InputLayout = { vertexElements.data(), static_cast<unsigned>(vertexElements.size()) };
			psoDesc.pRootSignature = mPBRRootSignature->mRootSignature->Get();
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

			ThrowIfFailed(engine->mDevice->CreateGraphicsPipelineState(
				&psoDesc, IID_PPV_ARGS(mPipelineState.GetAddressOf())));

			NAME_D3D12_OBJECT(mPipelineState);

		}
		catch (std::exception& e)
		{
			throw std::exception(e.what());
		}
	}

	void CDX12PBRPSO::Set(ID3D12GraphicsCommandList* commandList)
	{
		commandList->SetGraphicsRootSignature(mPBRRootSignature->mRootSignature->Get());
		mEngine->mCommandList->SetPipelineState(mPipelineState.Get());
	}

	CDX12SkyPSO::CDX12SkyPSO(CDX12Engine* engine) : mEngine(engine)
	{
		try
		{
			mRootSignature = std::make_unique<CDX12SkyRootSignature>(mEngine);

			// Check for presence of position and normal data. Tangents and UVs are optional.
			std::vector<D3D12_INPUT_ELEMENT_DESC> vertexElements;
			vertexElements.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			vertexElements.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			vertexElements.push_back({ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

			auto vs = engine->mBasicTransformVertexShader.get();
			auto ps = engine->mTintedTexturePixelShader.get();

			// Describe and create the graphics pipeline state object (PSO).
			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
			psoDesc.InputLayout = { vertexElements.data(), static_cast<unsigned>(vertexElements.size()) };
			psoDesc.pRootSignature = mRootSignature->mRootSignature->Get();
			psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs->mShaderBlob.Get());
			psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps->mShaderBlob.Get());
			psoDesc.RasterizerState = DirectX::CommonStates::CullNone;
			psoDesc.BlendState = DirectX::CommonStates::Opaque;
			psoDesc.DepthStencilState = DirectX::CommonStates::DepthNone;
			psoDesc.SampleMask = UINT_MAX;
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.NumRenderTargets = 1;
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			psoDesc.DepthStencilState.StencilEnable = FALSE;
			psoDesc.SampleDesc.Count = 1;

			ThrowIfFailed(engine->mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(mPipelineState.GetAddressOf())));

			NAME_D3D12_OBJECT(mPipelineState);
		}
		catch (std::exception& e)
		{
			throw std::exception(e.what());
		}

	}

	void CDX12SkyPSO::Set(ID3D12GraphicsCommandList* commandList)
	{
		commandList->SetGraphicsRootSignature(mRootSignature->mRootSignature->Get());
		mEngine->mCommandList->SetPipelineState(mPipelineState.Get());
		mEngine->SetConstantBuffers();
	}

	CDX12DepthOnlyPSO::CDX12DepthOnlyPSO(CDX12Engine* engine, bool requireTangents) : mEngine(engine)
	{
		try
		{
			mRootSignature = std::make_unique<CDX12DepthOnlyRootSignature>(mEngine);

			// Check for presence of position and normal data. Tangents and UVs are optional.
			std::vector<D3D12_INPUT_ELEMENT_DESC> vertexElements;
			vertexElements.push_back({ "position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			vertexElements.push_back({ "normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			if (requireTangents)
			{
				vertexElements.push_back({ "tangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			}

			vertexElements.push_back({ "uv", 0, DXGI_FORMAT_R32G32_FLOAT, 0, requireTangents ? 36u : 24u, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

			auto vs = requireTangents ? engine->mPbrNormalVertexShader.get() : engine->mPbrVertexShader.get();
			auto ps = requireTangents ? engine->mDepthOnlyNormalPixelShader.get() : engine->mDepthOnlyPixelShader.get();

			// Describe and create the graphics pipeline state object (PSO).
			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
			psoDesc.InputLayout = { vertexElements.data(), static_cast<unsigned>(vertexElements.size()) };
			psoDesc.pRootSignature = mRootSignature->mRootSignature->Get();
			psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs->mShaderBlob.Get());
			psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps->mShaderBlob.Get());
			psoDesc.RasterizerState = DirectX::CommonStates::CullNone;
			psoDesc.BlendState = DirectX::CommonStates::AlphaBlend;
			psoDesc.DepthStencilState = DirectX::CommonStates::DepthDefault;
			psoDesc.RasterizerState.SlopeScaledDepthBias = 1.f;
			psoDesc.RasterizerState.DepthBias = 100000;
			psoDesc.RasterizerState.DepthBiasClamp = 0.f;
			psoDesc.SampleMask = UINT_MAX;
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.NumRenderTargets = 0;
			psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
			psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			psoDesc.DepthStencilState.StencilEnable = FALSE;
			psoDesc.SampleDesc.Count = 1;

			ThrowIfFailed(engine->mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(mPipelineState.GetAddressOf())));

			NAME_D3D12_OBJECT(mPipelineState);
		}
		catch(const std::exception& e)
		{
			throw std::runtime_error(e.what());
		}
	}

	void CDX12DepthOnlyPSO::Set(ID3D12GraphicsCommandList* commandList)
	{
		commandList->SetGraphicsRootSignature(mRootSignature->mRootSignature->Get());
		mEngine->mCommandList->SetPipelineState(mPipelineState.Get());
	}
}
