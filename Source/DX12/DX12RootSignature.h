#pragma once
#include "DX12Engine.h"

namespace DX12
{

	class CDX12RootSignature
	{

	public:

<<<<<<< HEAD
<<<<<<< HEAD
		CDX12RootSignature(CDX12Engine* engine, CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc)
=======
		CDX12RootSignature(CDX12Engine* engine,
			D3D12_ROOT_SIGNATURE_FLAGS flags,
			UINT countOfRootParameters,
			CD3DX12_ROOT_PARAMETER1* rootParameters,
			D3D12_STATIC_SAMPLER_DESC samplerDesc)
>>>>>>> parent of 7bb1619 (Merge branch 'main' into TryingPolymorphism)
=======
		CDX12RootSignature(CDX12Engine* engine, CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc)
>>>>>>> parent of 5f2c2d1 (Working on IBL - DX12)
			:
			mEngine(engine)
		{
			//-----------------------------------
			// Create a root signature.
			D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
			if (FAILED(mEngine->mDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
			{
				featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
			}

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> parent of 5f2c2d1 (Working on IBL - DX12)
			// Serialize the root signature.
			ComPtr<ID3DBlob> rootSignatureBlob;
			ComPtr<ID3DBlob> errorBlob;
			D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &rootSignatureBlob, &errorBlob);
<<<<<<< HEAD
=======
			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
			rootSignatureDescription.Init_1_1(countOfRootParameters, rootParameters, 1u, &samplerDesc, flags);

			// Serialize the root signature.
			ComPtr<ID3DBlob> rootSignatureBlob;
			ComPtr<ID3DBlob> errorBlob;
			D3DX12SerializeVersionedRootSignature(&rootSignatureDescription, featureData.HighestVersion, &rootSignatureBlob, &errorBlob);
>>>>>>> parent of 7bb1619 (Merge branch 'main' into TryingPolymorphism)

			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}

			// Create the root signature.
			ThrowIfFailed(mEngine->mDevice->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
				rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));

=======

			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}

			// Create the root signature.
			ThrowIfFailed(mEngine->mDevice->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
				rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));

>>>>>>> parent of 5f2c2d1 (Working on IBL - DX12)
			NAME_D3D12_OBJECT(mRootSignature);
		}

		ID3D12RootSignature* Get() { return mRootSignature.Get(); }


	private:

		CDX12Engine* mEngine;
		ComPtr<ID3D12RootSignature> mRootSignature;

	};

	class CDX12PBRRootSignature
	{
	public:

		CDX12PBRRootSignature(CDX12Engine* engine)
			: mEngine(engine)
		{

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> parent of 5f2c2d1 (Working on IBL - DX12)
			// Allow input layout and deny unnecessary access to certain pipeline stages.
			D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

			constexpr auto numTextures = 6;
			constexpr auto numConstantBuffers = 6;
			constexpr auto totRanges = numConstantBuffers + numTextures + 2 ;

			// constant root parameters that are used by the vertex shader.
			CD3DX12_DESCRIPTOR_RANGE1 ranges[totRanges] = {};
			CD3DX12_ROOT_PARAMETER1 rootParameters[totRanges] = {};

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
<<<<<<< HEAD

				ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6);

				i++;
				ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7);
				

=======

				ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6);

				i++;
				ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7);
				

>>>>>>> parent of 5f2c2d1 (Working on IBL - DX12)
			}

			// Create root parameters
			{
				for (auto i = 0u; i < numConstantBuffers; ++i)
				{
						rootParameters[i].InitAsDescriptorTable(1, &ranges[i], D3D12_SHADER_VISIBILITY_ALL);
				}

				for (auto i = numConstantBuffers; i < totRanges; ++i)
				{
					rootParameters[i].InitAsDescriptorTable(1, &ranges[i],  D3D12_SHADER_VISIBILITY_PIXEL);
				}
			}

			
			D3D12_STATIC_SAMPLER_DESC samplers[] =
			{
				DirectX::CommonStates::StaticLinearWrap(0),
				DirectX::CommonStates::StaticAnisotropicClamp(1),
			};

			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
			rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, _countof(samplers), samplers, rootSignatureFlags);

			mRootSignature = std::make_unique<CDX12RootSignature>(engine, rootSignatureDescription);
		}

		CDX12Engine* mEngine;
		std::unique_ptr<CDX12RootSignature> mRootSignature;
	};


	class CDX12SkyRootSignature
	{
	public:

		~CDX12SkyRootSignature() = default;

		CDX12SkyRootSignature(CDX12Engine* engine)
			: mEngine(engine)
		{
<<<<<<< HEAD
=======
			enum
			{
				ModelCB,
				FrameCB,
				LightsCB,
				Albedo,
				Roughness,
				AO,
				Displacement,
				Normal,
				Metalness
			};

>>>>>>> parent of 7bb1619 (Merge branch 'main' into TryingPolymorphism)
=======
>>>>>>> parent of 5f2c2d1 (Working on IBL - DX12)
			// Allow input layout and deny unnecessary access to certain pipeline stages.
			D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

<<<<<<< HEAD
<<<<<<< HEAD

			constexpr auto numTextures = 1;
			constexpr auto numConstantBuffers = 6;
			constexpr auto totRanges = numConstantBuffers + numTextures;

			// constant root parameters that are used by the vertex shader.
			CD3DX12_DESCRIPTOR_RANGE1 ranges[totRanges] = {};
			CD3DX12_ROOT_PARAMETER1 rootParameters[totRanges] = {};

			// Create descriptor ranges
			{
				auto i = 0u;
				for (; i < numConstantBuffers; ++i)
				{
					ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, i);
				}

=======

			constexpr auto numTextures = 1;
			constexpr auto numConstantBuffers = 6;
			constexpr auto totRanges = numConstantBuffers + numTextures;

			// constant root parameters that are used by the vertex shader.
			CD3DX12_DESCRIPTOR_RANGE1 ranges[totRanges] = {};
			CD3DX12_ROOT_PARAMETER1 rootParameters[totRanges] = {};

			// Create descriptor ranges
			{
				auto i = 0u;
				for (; i < numConstantBuffers; ++i)
				{
					ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, i);
				}

>>>>>>> parent of 5f2c2d1 (Working on IBL - DX12)
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
					rootParameters[i].InitAsDescriptorTable(1, &ranges[i], D3D12_SHADER_VISIBILITY_ALL);
				}

				for (auto i = numConstantBuffers; i < totRanges; ++i)
				{
					rootParameters[i].InitAsDescriptorTable(1, &ranges[i], D3D12_SHADER_VISIBILITY_PIXEL);
				}
			}

			auto samplerDesc = DirectX::CommonStates::StaticAnisotropicBorder(0);

			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
			rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 1u, &samplerDesc, rootSignatureFlags);

			mRootSignature = std::make_unique<CDX12RootSignature>(engine, rootSignatureDescription);
		}

		CDX12Engine* mEngine;
		std::unique_ptr<CDX12RootSignature> mRootSignature;
	};

	class CDX12DepthOnlyRootSignature
	{
	public:

		~CDX12DepthOnlyRootSignature() = default;

		CDX12DepthOnlyRootSignature(CDX12Engine * engine)
			: mEngine(engine)
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
			CD3DX12_ROOT_PARAMETER1 rootParameters[totRanges] = {};

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
					rootParameters[i].InitAsDescriptorTable(1, &ranges[i], D3D12_SHADER_VISIBILITY_ALL);
				}

				for (auto i = numConstantBuffers; i < totRanges; ++i)
				{
					rootParameters[i].InitAsDescriptorTable(1, &ranges[i], D3D12_SHADER_VISIBILITY_PIXEL);
				}
			}

			auto samplerDesc = DirectX::CommonStates::StaticPointClamp(0);

			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
			rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 1u, &samplerDesc, rootSignatureFlags);
<<<<<<< HEAD

			mRootSignature = std::make_unique<CDX12RootSignature>(engine, rootSignatureDescription);
=======
			// constant root parameters that are used by the vertex shader.
			CD3DX12_DESCRIPTOR_RANGE1 ranges[9] = {};
			CD3DX12_ROOT_PARAMETER1 rootParameters[9] = {};

			ranges[ModelCB].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // per model constant buffer
			ranges[FrameCB].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1); // per frame constant buffer
			ranges[LightsCB].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2); // per lights constant buffer

			ranges[Albedo].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // texture
			ranges[Roughness].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1); // texture
			ranges[AO].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2); // texture
			ranges[Displacement].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3); // texture
			ranges[Normal].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4); // texture
			ranges[Metalness].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5); // texture


			rootParameters[ModelCB].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
			rootParameters[FrameCB].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);
			rootParameters[LightsCB].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_ALL);


			rootParameters[Albedo].InitAsDescriptorTable(1, &ranges[3], D3D12_SHADER_VISIBILITY_PIXEL);
			rootParameters[Roughness].InitAsDescriptorTable(1, &ranges[4], D3D12_SHADER_VISIBILITY_PIXEL);
			rootParameters[AO].InitAsDescriptorTable(1, &ranges[5], D3D12_SHADER_VISIBILITY_PIXEL);
			rootParameters[Displacement].InitAsDescriptorTable(1, &ranges[6], D3D12_SHADER_VISIBILITY_PIXEL);
			rootParameters[Normal].InitAsDescriptorTable(1, &ranges[7], D3D12_SHADER_VISIBILITY_PIXEL);
			rootParameters[Metalness].InitAsDescriptorTable(1, &ranges[8], D3D12_SHADER_VISIBILITY_PIXEL);

			auto samplerDesc = DirectX::CommonStates::StaticAnisotropicWrap(0);

			mRootSignature = std::make_unique<CDX12RootSignature>(engine, rootSignatureFlags, 9, rootParameters, samplerDesc);
>>>>>>> parent of 7bb1619 (Merge branch 'main' into TryingPolymorphism)
=======

			mRootSignature = std::make_unique<CDX12RootSignature>(engine, rootSignatureDescription);
>>>>>>> parent of 5f2c2d1 (Working on IBL - DX12)
		}

		CDX12Engine* mEngine;
		std::unique_ptr<CDX12RootSignature> mRootSignature;
	};
}