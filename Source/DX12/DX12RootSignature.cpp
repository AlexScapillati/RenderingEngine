#include "DX12RootSignature.h"

#include "CommonStates.h"

#include "DX12Engine.h"

namespace DX12
{
	void CDX12RootSignature::Set(ID3D12RootSignature* rootSignature)
	{
		mRootSignature = rootSignature;
	}

	CDX12RootSignature::CDX12RootSignature(CDX12Engine* engine, CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc) :
		mEngine(engine)
	{

		std::unique_lock l(engine->mMutex);

		//-----------------------------------
		// Create a root signature.
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if (FAILED(mEngine->mDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		// Serialize the root signature.
		ComPtr<ID3DBlob> rootSignatureBlob;
		ComPtr<ID3DBlob> errorBlob;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &rootSignatureBlob, &errorBlob));

		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}

		// Create the root signature.
		ThrowIfFailed(mEngine->mDevice->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
			rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));

		NAME_D3D12_OBJECT(mRootSignature);
	}

	ID3D12RootSignature* CDX12RootSignature::Get()
	{
		return mRootSignature.Get();
	}

	CDX12PBRRootSignature::CDX12PBRRootSignature(CDX12Engine* engine) : mEngine(engine)
	{

		// Allow input layout and deny unnecessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		constexpr auto numTextures = 8;
		constexpr auto numConstantBuffers = 6;

		// constant root parameters that are used by the vertex shader.
		CD3DX12_DESCRIPTOR_RANGE1 ranges[numTextures + numConstantBuffers] = {};
		CD3DX12_ROOT_PARAMETER1   rootParameters[numTextures + numConstantBuffers] = {};

		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 3, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 4, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		ranges[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 5, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

		ranges[ 6].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		ranges[ 7].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		ranges[ 8].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		ranges[ 9].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		ranges[10].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		ranges[11].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		ranges[12].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		ranges[13].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

		// Create root parameters
		// CB
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0]);
		rootParameters[1].InitAsDescriptorTable(1, &ranges[1]);
		rootParameters[2].InitAsDescriptorTable(1, &ranges[2]);
		rootParameters[3].InitAsDescriptorTable(1, &ranges[3]);
		rootParameters[4].InitAsDescriptorTable(1, &ranges[4]);
		rootParameters[5].InitAsDescriptorTable(1, &ranges[5]);

		// SRV
		rootParameters[6].InitAsDescriptorTable(1, &ranges[6], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[7].InitAsDescriptorTable(1, &ranges[7], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[8].InitAsDescriptorTable(1, &ranges[8], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[9].InitAsDescriptorTable(1, &ranges[9], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[10].InitAsDescriptorTable(1, &ranges[10], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[11].InitAsDescriptorTable(1, &ranges[11], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[12].InitAsDescriptorTable(1, &ranges[12], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[13].InitAsDescriptorTable(1, &ranges[13], D3D12_SHADER_VISIBILITY_PIXEL);


		D3D12_STATIC_SAMPLER_DESC samplers[] =
		{
			DirectX::CommonStates::StaticLinearWrap(0),
			DirectX::CommonStates::StaticAnisotropicClamp(1),
		};

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
		rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, _countof(samplers), samplers, rootSignatureFlags);

		mRootSignature = std::make_unique<CDX12RootSignature>(engine, rootSignatureDescription);

	}


	CDX12SkyRootSignature::CDX12SkyRootSignature(CDX12Engine* engine) : mEngine(engine)
	{
		// Allow input layout and deny unnecessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		constexpr auto numTextures = 1;
		constexpr auto numConstantBuffers = 6;

		// constant root parameters that are used by the vertex shader.
		CD3DX12_DESCRIPTOR_RANGE1 ranges[numTextures + numConstantBuffers] = {};
		CD3DX12_ROOT_PARAMETER1   rootParameters[numTextures + numConstantBuffers] = {};

		// Create descriptor ranges

		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 3, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 4, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		ranges[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 5, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

		ranges[6].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

		// Create root parameters
		// CB
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0]);
		rootParameters[1].InitAsDescriptorTable(1, &ranges[1]);
		rootParameters[2].InitAsDescriptorTable(1, &ranges[2]);
		rootParameters[3].InitAsDescriptorTable(1, &ranges[3]);
		rootParameters[4].InitAsDescriptorTable(1, &ranges[4]);
		rootParameters[5].InitAsDescriptorTable(1, &ranges[5]);

		// SRV
		rootParameters[6].InitAsDescriptorTable(1, &ranges[6], D3D12_SHADER_VISIBILITY_PIXEL);

		auto samplerDesc = DirectX::CommonStates::StaticPointClamp(0);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
		rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 1u, &samplerDesc, rootSignatureFlags);

		mRootSignature = std::make_unique<CDX12RootSignature>(engine, rootSignatureDescription);
	}

	CDX12DepthOnlyRootSignature::CDX12DepthOnlyRootSignature(CDX12Engine* engine) : mEngine(engine)
	{
		// Allow input layout and deny unnecessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;


		constexpr auto numTextures = 1;
		constexpr auto numConstantBuffers = 6;
		constexpr auto totRanges = numConstantBuffers + numTextures;

		// constant root parameters that are used by the vertex shader.
		CD3DX12_DESCRIPTOR_RANGE1 ranges[numTextures + numConstantBuffers] = {};
		CD3DX12_ROOT_PARAMETER1   rootParameters[numTextures + numConstantBuffers] = {};

		// Create descriptor ranges

		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
		ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 3);
		ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 4);
		ranges[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 5);

		ranges[6].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		// Create root parameters
		// CB
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0]);
		rootParameters[1].InitAsDescriptorTable(1, &ranges[1]);
		rootParameters[2].InitAsDescriptorTable(1, &ranges[2]);
		rootParameters[3].InitAsDescriptorTable(1, &ranges[3]);
		rootParameters[4].InitAsDescriptorTable(1, &ranges[4]);
		rootParameters[5].InitAsDescriptorTable(1, &ranges[5]);

		// SRV
		rootParameters[6].InitAsDescriptorTable(1, &ranges[6]);


		auto samplerDesc = DirectX::CommonStates::StaticPointClamp(0);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
		rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 1u, &samplerDesc, rootSignatureFlags);

		mRootSignature = std::make_unique<CDX12RootSignature>(engine, rootSignatureDescription);
	}
}
