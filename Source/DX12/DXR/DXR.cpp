
#include "DXR.h"

#include <CommonStates.h>

#include "../../Window.h"
#include "../../Common/CGameObjectManager.h"

#include "../Objects/DX12GameObject.h"

#include "BottomLevelASGenerator.h"
#include "RaytracingPipelineGenerator.h"
#include "RootSignatureGenerator.h"
#include "ShaderBindingTableGenerator.h"
#include "TopLevelASGenerator.h"

#include "../DX12ConstantBuffer.h"
#include "../DX12DescriptorHeap.h"
#include "../DX12Shader.h"
#include "../DX12Texture.h"
#include "../DX12Scene.h"

namespace DX12
{


	ComPtr<ID3D12RootSignature> CreateRayGenSignature(ID3D12Device* device)
	{
		DXR::RootSignatureGenerator rsc;
		rsc.AddHeapRangesParameter({
			{0 /*u0*/, 1 /*1 descriptor */, 0 /*use the implicit register space 0*/,D3D12_DESCRIPTOR_RANGE_TYPE_UAV /* UAV representing the output buffer*/,0 /*heap slot where the UAV is defined*/},
			{0 /*t0*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV /*Top-level acceleration structure*/,1},
			{0 /*b0*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_CBV /*Camera parameters*/,2} });

		return rsc.Generate(device, true, {});
	}

	ComPtr<ID3D12RootSignature>CreateMissSignature(ID3D12Device* device)
	{
		DXR::RootSignatureGenerator rsc;
		return rsc.Generate(device, true, {});
	}

	ComPtr<ID3D12RootSignature> CreateHitSignature(ID3D12Device* device)
	{
		DXR::RootSignatureGenerator rsc;

		rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_CBV, 0); // Lights Buffer
		rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_CBV, 1); // Texture dimensions buffer

		rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_SRV);// Acceleration structure
		rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_SRV, 1); // Vertex buffer			
		rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_SRV, 2); // Index buffer

		rsc.AddHeapRangesParameter({ { 3, 6, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV ,0 } }); // Textures

		return rsc.Generate(device, true, {});
	}


	ComPtr<ID3D12RootSignature> CreateShadowSignature(ID3D12Device* device)
	{
		DXR::RootSignatureGenerator rsc;
		return rsc.Generate(device, true, {});
	}

	AccelerationStructureBuffers CreateBottomLevelAS(
		CDX12Engine* engine,
		std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vVertexBuffers,
		uint32_t vertexSize,
		std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vIndexBuffers,
		std::vector<ComPtr<ID3D12Resource>> vTransformMatrices
	)
	{
		auto device = engine->mDevice.Get();
		auto commandList = engine->GetCommandList();

		DXR::BottomLevelASGenerator bottomLevelAS;
		// Adding all vertex buffers and not transforming their position.
		for (size_t i = 0; i < vVertexBuffers.size(); i++)
		{
			bottomLevelAS.AddVertexBuffer(
				vVertexBuffers[i].first.Get(),
				0,
				vVertexBuffers[i].second,
				vertexSize,
				vIndexBuffers[i].first.Get(),
				0,
				vIndexBuffers[i].second,
				!vTransformMatrices.empty() ? vTransformMatrices[i].Get() : nullptr,
				0,
				true);
		}

		// The AS build requires some scratch space to store temporary information.
		// The amount of scratch memory is dependent on the scene complexity.
		UINT64 scratchSizeInBytes = 0;
		// The final AS also needs to be stored in addition to the existing vertex
		// buffers. It size is also dependent on the scene complexity.
		UINT64 resultSizeInBytes = 0;

		bottomLevelAS.ComputeASBufferSizes(device, false, &scratchSizeInBytes,
			&resultSizeInBytes);

		// Once the sizes are obtained, the application is responsible for allocating
		// the necessary buffers. Since the entire generation will be done on the GPU,
		// we can directly allocate those on the default heap
		AccelerationStructureBuffers buffers;
		buffers.pScratch = DXR::CreateBuffer(
			device, scratchSizeInBytes,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON,
			DXR::kDefaultHeapProps);
		buffers.pResult = DXR::CreateBuffer(
			device, resultSizeInBytes,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			DXR::kDefaultHeapProps);

		// Build the acceleration structure. Note that this call integrates a barrier
		// on the generated AS, so that it can be used to compute a top-level AS right
		// after this method.
		bottomLevelAS.Generate(commandList, buffers.pScratch.Get(),
			buffers.pResult.Get(), false, nullptr);

		return buffers;
	}


	void CreateTopLevelAS(std::vector<std::pair<ComPtr<ID3D12Resource>, CMatrix4x4*>>& instances, CDX12Engine* engine, bool updateOnly)
	{
		auto device = engine->mDevice.Get();
		auto TLASG = engine->mTopLevelAsGenerator.get();
		auto TLASB = &engine->mTopLevelAsBuffers;
		auto commandList = engine->GetCommandList();

		if (!updateOnly)
		{
			// Gather all the instances into the builder helper
			for (size_t i = 0; i < instances.size(); ++i)
			{
				TLASG->AddInstance(instances[i].first.Get(), *instances[i].second, static_cast<UINT>(i), static_cast<UINT>(0));
			}
			// As for the bottom-level AS, the building the AS requires some scratch space
			// to store temporary data in addition to the actual AS. In the case of the
			// top-level AS, the instance descriptors also need to be stored in GPU
			// memory. This call outputs the memory requirements for each (scratch,
			// results, instance descriptors) so that the application can allocate the
			// corresponding memory
			UINT64 scratchSize, resultSize, instanceDescsSize;
			TLASG->ComputeASBufferSizes(device, true, &scratchSize, &resultSize, &instanceDescsSize);
			// Create the scratch and result buffers. Since the build is all done on GPU,
			// those can be allocated on the default heap
			TLASB->pScratch = DXR::CreateBuffer(device, scratchSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, DXR::kDefaultHeapProps);
			TLASB->pResult = DXR::CreateBuffer(device, resultSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, DXR::kDefaultHeapProps);
			// The buffer describing the instances: ID, shader binding information,
			// matrices ... Those will be copied into the buffer by the helper through
			// mapping, so the buffer has to be allocated on the upload heap.
			TLASB->pInstanceDesc = DXR::CreateBuffer(device, instanceDescsSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, DXR::kUploadHeapProps);
		}

		// After all the buffers are allocated, or if only an update is required, we
		// can build the acceleration structure. Note that in the case of the update
		// we also pass the existing AS as the 'previous' AS, so that it can be
		// refitted in place.
		TLASG->Generate(commandList, TLASB->pScratch.Get(), TLASB->pResult.Get(), TLASB->pInstanceDesc.Get(), updateOnly, TLASB->pResult.Get());
	}

	void CreateAccelerationStructures(CDX12Engine* engine)
	{

		engine->mInstances = {};

		AccelerationStructureBuffers bottomLevelBuffers;
		uint32_t vertexSize = 0;

		// Get the vertex and index buffers and put them in a vector
		std::vector<ComPtr<ID3D12Resource>> vTransformMatrices;

		if (engine->GetObjManager())
		{
			for (const auto& object : engine->GetObjManager()->mObjects)
			{
				auto o = dynamic_cast<CDX12GameObject*>(object);

				auto mesh = o->Mesh();
				auto& subMeshes = mesh->mSubMeshes;
				for (auto i = 0; i < subMeshes.size(); ++i)
				{
					std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vertexBuffers;
					std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> indexBuffers;

					vertexBuffers.emplace_back(subMeshes[i].mVertexBuffer, subMeshes[i].numVertices);
					indexBuffers.emplace_back(subMeshes[i].mIndexBuffer, subMeshes[i].numIndices);
					vertexSize = subMeshes[i].vertexSize;

					// Build the bottom AS from the Triangle vertex buffer
					bottomLevelBuffers = CreateBottomLevelAS(engine, vertexBuffers, vertexSize, indexBuffers, vTransformMatrices);
					engine->mInstances.emplace_back(bottomLevelBuffers.pResult, &o->WorldMatrix());
				}
			}
		}

		CreateTopLevelAS(engine->mInstances, engine);

		// Flush the command list and wait for it to finish
		auto commandList = engine->GetCommandList();

		commandList->Close();
		ID3D12CommandList* ppCommandLists[] = { commandList };
		engine->mCommandQueue->ExecuteCommandLists(1, ppCommandLists);

		// Wait for GPU to finish executing command list
		engine->Flush();
	}


}
