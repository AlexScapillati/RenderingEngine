#include "DX12RootSignature.h"

<<<<<<< HEAD
#include "CommonStates.h"

#include "DX12Engine.h"
#include "DXR/RootSignatureGenerator.h"

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

=======
#include "DX12Engine.h"

namespace DX12
{
	CDX12RootSignature::CDX12RootSignature(CDX12Engine* engine, CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc) :
		mEngine(engine)
	{
>>>>>>> parent of a9c1de14 (revert commit)
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
		D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &rootSignatureBlob, &errorBlob);

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

		constexpr auto numTextures = 6;
		constexpr auto numConstantBuffers = 6;
		constexpr auto totRanges = numConstantBuffers + numTextures + 2;

		// constant root parameters that are used by the vertex shader.
		CD3DX12_DESCRIPTOR_RANGE1 ranges[totRanges] = {};
		CD3DX12_ROOT_PARAMETER1   rootParameters[totRanges] = {};

		// Create descriptor ranges
		{
			auto i = 0u;
			for (; i < numConstantBuffers; ++i)
			{
				ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, i);
			}

			for (auto j = 0u; j < numTextures; ++j)
			{
				ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, j);
				i++;
			}

			ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6);

			i++;
			ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7);
		}

		// Create root parameters
		{
			for (auto i = 0u; i < numConstantBuffers; ++i)
			{
<<<<<<< HEAD
				rootParameters[i].InitAsDescriptorTable(1,&ranges[i]);
=======
				rootParameters[i].InitAsDescriptorTable(1, &ranges[i], D3D12_SHADER_VISIBILITY_ALL);
>>>>>>> parent of a9c1de14 (revert commit)
			}

			for (auto i = numConstantBuffers; i < totRanges; ++i)
			{
				rootParameters[i].InitAsDescriptorTable(1, &ranges[i], D3D12_SHADER_VISIBILITY_PIXEL);
			}
		}

<<<<<<< HEAD
=======

>>>>>>> parent of a9c1de14 (revert commit)
		D3D12_STATIC_SAMPLER_DESC samplers[] =
		{
			DirectX::CommonStates::StaticLinearWrap(0),
			DirectX::CommonStates::StaticAnisotropicClamp(1),
		};

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
		rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, _countof(samplers), samplers, rootSignatureFlags);

		mRootSignature = std::make_unique<CDX12RootSignature>(engine, rootSignatureDescription);
<<<<<<< HEAD
		
=======
>>>>>>> parent of a9c1de14 (revert commit)
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
		constexpr auto totRanges = numConstantBuffers + numTextures;

		// constant root parameters that are used by the vertex shader.
		CD3DX12_DESCRIPTOR_RANGE1 ranges[totRanges] = {};
		CD3DX12_ROOT_PARAMETER1   rootParameters[totRanges] = {};

		// Create descriptor ranges
		{
			auto i = 0u;
			for (; i < numConstantBuffers; ++i)
			{
				ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, i);
			}

			for (auto j = 0u; j < numTextures; ++j)
			{
				ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, j);
				++i;
			}
		}

		// Create root parameters
		{
			for (auto i = 0u; i < numConstantBuffers; ++i)
			{
<<<<<<< HEAD
				rootParameters[i].InitAsDescriptorTable(1, &ranges[i]);
=======
				rootParameters[i].InitAsDescriptorTable(1, &ranges[i], D3D12_SHADER_VISIBILITY_ALL);
>>>>>>> parent of a9c1de14 (revert commit)
			}

			for (auto i = numConstantBuffers; i < totRanges; ++i)
			{
				rootParameters[i].InitAsDescriptorTable(1, &ranges[i], D3D12_SHADER_VISIBILITY_PIXEL);
			}
		}

<<<<<<< HEAD
		static const D3D12_STATIC_SAMPLER_DESC s = {
		 D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
		 D3D12_TEXTURE_ADDRESS_MODE_BORDER, // AddressU
		 D3D12_TEXTURE_ADDRESS_MODE_BORDER, // AddressV
		 D3D12_TEXTURE_ADDRESS_MODE_BORDER, // AddressW
		 0, // MipLODBias
		 D3D12_MAX_MAXANISOTROPY,
		 D3D12_COMPARISON_FUNC_NEVER,
		 D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK,
		 0, // MinLOD
		 FLT_MAX, // MaxLOD
		 0, // ShaderRegister
		 0, // RegisterSpace
		 D3D12_SHADER_VISIBILITY_ALL,
		};

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
		rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 1u, &s, rootSignatureFlags);
=======
		auto samplerDesc = DirectX::CommonStates::StaticAnisotropicBorder(0);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
		rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 1u, &samplerDesc, rootSignatureFlags);
>>>>>>> parent of a9c1de14 (revert commit)

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


		constexpr auto numTextures        = 1;
		constexpr auto numConstantBuffers = 6;
		constexpr auto totRanges          = numConstantBuffers + numTextures;

		// constant root parameters that are used by the vertex shader.
		CD3DX12_DESCRIPTOR_RANGE1 ranges[totRanges]         = {};
		CD3DX12_ROOT_PARAMETER1   rootParameters[totRanges] = {};

		// Create descriptor ranges
		{
			auto i = 0u;
			for (; i < numConstantBuffers; ++i)
			{
				ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, i);
			}

			for (auto j = 0u; j < numTextures; ++j)
			{
				ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, j);
				++i;
			}
		}

		// Create root parameters
		{
			for (auto i = 0u; i < numConstantBuffers; ++i)
			{
<<<<<<< HEAD
				rootParameters[i].InitAsDescriptorTable(1, &ranges[i]);
=======
				rootParameters[i].InitAsDescriptorTable(1, &ranges[i], D3D12_SHADER_VISIBILITY_ALL);
>>>>>>> parent of a9c1de14 (revert commit)
			}

			for (auto i = numConstantBuffers; i < totRanges; ++i)
			{
				rootParameters[i].InitAsDescriptorTable(1, &ranges[i], D3D12_SHADER_VISIBILITY_PIXEL);
			}
		}

		auto samplerDesc = DirectX::CommonStates::StaticPointClamp(0);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
		rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 1u, &samplerDesc, rootSignatureFlags);

		mRootSignature = std::make_unique<CDX12RootSignature>(engine, rootSignatureDescription);
	}
}
