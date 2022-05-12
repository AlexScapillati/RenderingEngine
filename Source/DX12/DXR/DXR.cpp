
#include "DXR.h"

#include "BottomLevelASGenerator.h"
#include "../DX12Engine.h"

#include "RaytracingPipelineGenerator.h"
#include "RootSignatureGenerator.h"
#include "ShaderBindingTableGenerator.h"
#include "TopLevelASGenerator.h"
#include "../DX12ConstantBuffer.h"
#include "../DX12DescriptorHeap.h"
#include "../DX12Shader.h"
#include "../DX12Texture.h"
#include "../../Window.h"
#include "../../Common/CGameObjectManager.h"
#include "../../Common/CScene.h"
#include "../Objects/DX12GameObject.h"

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

		rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_SRV, 0); // Vertex buffer			
		rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_SRV, 1); // Index buffer

		rsc.AddHeapRangesParameter(
			{
				{2,1,0,D3D12_DESCRIPTOR_RANGE_TYPE_SRV,2}    // Lights buffer
			});

		//rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_SRV, 2);

		return rsc.Generate(device, true, {});
	}

	AccelerationStructureBuffers CreateBottomLevelAS(
		CDX12Engine* engine,
		std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vVertexBuffers,
		uint32_t vertexSize,
		std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vIndexBuffers
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
				nullptr,
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
		AccelerationStructureBuffers bottomLevelBuffers;
		uint32_t vertexSize = 0;

		for (const auto& object : engine->GetObjManager()->mObjects)
		{
			if (const auto o = dynamic_cast<CDX12GameObject*>(object)) // Cheating
			{

				// Get the vertex and index buffers and put them in a vector
				std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vertexBuffers;
				std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> indexBuffers;

				for (const auto& subMesh : o->Mesh()->mSubMeshes)
				{
					vertexBuffers.emplace_back(subMesh.mVertexBuffer, subMesh.numVertices);
					indexBuffers.emplace_back(subMesh.mIndexBuffer, subMesh.numIndices);

					vertexSize = subMesh.vertexSize;
				}

				// Build the bottom AS from the Triangle vertex buffer
				bottomLevelBuffers = CreateBottomLevelAS(engine, vertexBuffers, vertexSize, indexBuffers);
				engine->mInstances.emplace_back(bottomLevelBuffers.pResult, &object->WorldMatrix());
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

	void CDX12Engine::CreateRaytracingPipeline()
	{
		auto device = mDevice.Get();

		mRayTracingPipeline = std::make_unique<DXR::RayTracingPipelineGenerator>(mDevice.Get());
		// The pipeline contains the DXIL code of all the shaders potentially executed
		 // during the raytracing process. This section compiles the HLSL code into a
		 // set of DXIL libraries. We chose to separate the code in several libraries
		 // by semantic (ray generation, hit, miss) for clarity. Any code layout can be
		 // used.

		const auto target = L"-T lib_6_3";

		std::vector args{ target };
		mRayGenLibrary = CompileShader(L"RayGen.hlsl", args);
		mMissLibrary = CompileShader(L"Miss.hlsl", args);
		mHitLibrary = CompileShader(L"Hit.hlsl", args);

		// In a way similar to DLLs, each library is associated with a number of
		// exported symbols. This
		// has to be done explicitly in the lines below. Note that a single library
		// can contain an arbitrary number of symbols, whose semantic is given in HLSL
		// using the [shader("xxx")] syntax
		mRayTracingPipeline->AddLibrary(mRayGenLibrary.Get(), { L"RayGen" });
		mRayTracingPipeline->AddLibrary(mMissLibrary.Get(), { L"Miss" });
		// #DXR Extra: PerInstance Data
		mRayTracingPipeline->AddLibrary(mHitLibrary.Get(), { L"ClosestHit" });


		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//	 As described at the beginning of this section, to each shader corresponds a root signature defining
		//	 its external inputs.
		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// To be used, each DX12 shader needs a root signature defining which
		// parameters and buffers will be accessed.
		mRayGenSignature = CreateRayGenSignature(device);
		mMissSignature = CreateMissSignature(device);
		mHitSignature = CreateHitSignature(device);



		// 3 different shaders can be invoked to obtain an intersection: an
		// intersection shader is called
		// when hitting the bounding box of non-triangular geometry. This is beyond
		// the scope of this tutorial. An any-hit shader is called on potential
		// intersections. This shader can, for example, perform alpha-testing and
		// discard some intersections. Finally, the closest-hit program is invoked on
		// the intersection point closest to the ray origin. Those 3 shaders are bound
		// together into a hit group.

		// Note that for triangular geometry the intersection shader is built-in. An
		// empty any-hit shader is also defined by default, so in our simple case each
		// hit group contains only the closest hit shader. Note that since the
		// exported symbols are defined above the shaders can be simply referred to by
		// name.

		// Hit group for the triangles, with a shader simply interpolating vertex
		// colors
		mRayTracingPipeline->AddHitGroup(L"HitGroup", L"ClosestHit");

		//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//	To be used, each shader needs to be associated to its root signature.A shaders imported from the DXIL
		//	libraries needs to be associated with exactly one root signature.The shaders comprising the hit groups
		//	need to share the same root signature, which is associated to the hit group(and not to the shaders themselves).
		//	Note that a shader does not have to actually access all the resources declared in its root signature,
		//	as long as the root signature defines a superset of the resources the shader needs.
		//	 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// The following section associates the root signature to each shader. Note
		// that we can explicitly show that some shaders share the same root signature
		// (eg. Miss and ShadowMiss). Note that the hit shaders are now only referred
		// to as hit groups, meaning that the underlying intersection, any-hit and
		// closest-hit shaders share the same root signature.
		mRayTracingPipeline->AddRootSignatureAssociation(mRayGenSignature.Get(), { L"RayGen" });
		mRayTracingPipeline->AddRootSignatureAssociation(mMissSignature.Get(), { L"Miss" });
		mRayTracingPipeline->AddRootSignatureAssociation(mHitSignature.Get(), { L"HitGroup" });

		// The payload size defines the maximum size of the data carried by the rays,
		// ie. the the data
		// exchanged between shaders, such as the HitInfo structure in the HLSL code.
		// It is important to keep this value as low as possible as a too high value
		// would result in unnecessary memory consumption and cache trashing.
		mRayTracingPipeline->SetMaxPayloadSize(4 * sizeof(float)); // RGB + distance

		// Upon hitting a surface, DXR can provide several attributes to the hit. In
		// our sample we just use the barycentric coordinates defined by the weights
		// u,v of the last two vertices of the triangle. The actual barycentrics can
		// be obtained using float3 barycentrics = float3(1.f-u-v, u, v);
		mRayTracingPipeline->SetMaxAttributeSize(2 * sizeof(float)); // barycentric coordinates

		// The raytracing process can shoot rays from existing hit points, resulting
		// in nested TraceRay calls. Our sample code traces only primary rays, which
		// then requires a trace depth of 1. Note that this recursion depth should be
		// kept to a minimum for best performance. Path tracing algorithms can be
		// easily flattened into a simple loop in the ray generation.
		mRayTracingPipeline->SetMaxRecursionDepth(1);

		// Compile the pipeline for execution on the GPU
		mRaytracingStateObject = mRayTracingPipeline->Generate();

		// Cast the state object into a properties object, allowing to later access
		// the shader pointers by name
		ThrowIfFailed(mRaytracingStateObject->QueryInterface(IID_PPV_ARGS(mRaytracingStateObjectProps.GetAddressOf())));
	}

	void CDX12Engine::InitRaytracing()
	{

		InitializeFrame();

		mTopLevelAsGenerator = std::make_unique<DXR::TopLevelASGenerator>();

		CreateAccelerationStructures(this);

		CreateRaytracingPipeline();

		// Create a SRV/UAV/CBV descriptor heap. We need 3 entries -
		// 1 UAV for the raytracing output
		// 1 SRV for the TLAS
		// 1 UAV for the CBuffer
		mRTHeap = std::make_unique<CDX12DescriptorHeap>(this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4 * mNumFrames, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

		// Create the output texture

		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.DepthOrArraySize = 1;
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		// The backbuffer is actually DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, but sRGB
		// formats cannot be used with UAVs. For accuracy we should convert to sRGB
		// ourselves in the shader
		resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		resDesc.Width = GetWindow()->GetWindowWidth();
		resDesc.Height = GetWindow()->GetWindowHeight();
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resDesc.MipLevels = 1;
		resDesc.SampleDesc.Count = 1;

		mDevice->CreateCommittedResource(&DXR::kDefaultHeapProps, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr, IID_PPV_ARGS(mOutputResource.GetAddressOf()));

		mOutputSrvIndex = mRTHeap->mHandles.size();
		auto handle = mRTHeap->Get(mRTHeap->Add());

		// Get the CPU handle of the output texture present in the RT heap
		auto srvHandle = handle->mCpu;

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		mDevice->CreateUnorderedAccessView(mOutputResource.Get(), nullptr, &uavDesc, srvHandle);

		mRTHeap->Add();

		// Add the Top Level AS SRV right after the raytracing output buffer
		srvHandle.ptr += mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc; srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.RaytracingAccelerationStructure.Location = mTopLevelAsBuffers.pResult->GetGPUVirtualAddress();
		// Write the acceleration structure view in the heap
		mDevice->CreateShaderResourceView(nullptr, &srvDesc, srvHandle);

		// #DXR Extra: Perspective Camera
		// Add the constant buffer for the camera after the TLAS
		srvHandle.ptr += mDevice->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		for (int i = 0; i < mNumFrames; ++i)
		{
			mCameraBuffer[i] = std::make_unique<CDX12ConstantBuffer>(this, mRTHeap.get(), 4 * sizeof(CMatrix4x4));

			mRTLightsBuffer[i] = std::make_unique<CDX12ConstantBuffer>(this, mRTHeap.get(), sizeof(PerFrameLights));
			mRTLightsBuffer[i]->Copy(mPerFrameLights);
			mRTLightsBuffer[i]->Resource()->SetName(L"RaytracingLightsBuffer");
		}

		CreateShaderBindingTable();
	}

	void CDX12Engine::CreateShaderBindingTable()
	{
		// The SBT helper class collects calls to Add*Program. If called several
		// times, the helper must be emptied before re-adding shaders.

		mSbtHelper = std::make_unique<DXR::ShaderBindingTableGenerator>();

		mSbtHelper->Reset();
		// The pointer to the beginning of the heap is the only parameter required by
		// shaders without root parameters
		D3D12_GPU_DESCRIPTOR_HANDLE srvUavHeapHandle = mRTHeap->mDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

		auto heapPointer = reinterpret_cast<void*>(srvUavHeapHandle.ptr);

		/*
			We can now add the various programs used in our example : according to its root signature, the ray generation shader needs to access
			the raytracing output bufferand the top - level acceleration structure referenced in the heap.Therefore, it
			takes a single resource pointer towards the beginning of the heap data.The miss shaderand the hit group
			only communicate through the ray payload, and do not require any resource, hence an empty resource array.
			Note that the helper will group the shaders by types in the SBT, so it is possible to declare them in an
			arbitrary order.For example, miss programs can be added before or after ray generation programs without
			affecting the result.
			However, within a given type(say, the hit groups), the order in which they are added
			is important.It needs to correspond to the `InstanceContributionToHitGroupIndex` value used when adding
			instances to the top - level acceleration structure : for example, an instance having `InstanceContributionToHitGroupIndex= = 0`
			needs to have its hit group added first in the SBT.
		 */

		 // The ray generation only uses heap data
		mSbtHelper->AddRayGenerationProgram(L"RayGen", { heapPointer });
		// The miss and hit shaders do not access any external resources: instead they
		// communicate their results through the ray payload
		mSbtHelper->AddMissProgram(L"Miss", {});

		std::vector<void*> inputData;

		for (const auto& object : GetObjManager()->mObjects)
		{
			if (const auto o = dynamic_cast<CDX12GameObject*>(object))
			{
				for (const auto& subMesh : o->Mesh()->mSubMeshes)
				{
					inputData.push_back((void*)subMesh.mVertexBuffer->GetGPUVirtualAddress());
					inputData.push_back((void*)subMesh.mIndexBuffer->GetGPUVirtualAddress());
				}
			}
		}

		// Adding the triangle hit shader
		mSbtHelper->AddHitGroup(L"HitGroup", inputData);

		// Compute the size of the SBT given the number of shaders and their
		// parameters
		uint32_t sbtSize = mSbtHelper->ComputeSBTSize();

		// Create the SBT on the upload heap. This is required as the helper will use
		// mapping to write the SBT contents. After the SBT compilation it could be
		// copied to the default heap for performance.
		mSbtStorage = DXR::CreateBuffer(mDevice.Get(), sbtSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, DXR::kUploadHeapProps);

		if (!mSbtStorage)
		{
			throw std::logic_error("Could not allocate the shader binding table");
		}

		// Compile the SBT from the shader and parameters info
		mSbtHelper->Generate(mSbtStorage.Get(), mRaytracingStateObjectProps.Get());
	}

	void CDX12Engine::RaytracingFrame()
	{

		// Copy and set constant buffers

		auto c = mScene->GetCamera();

		CMatrix4x4 m[] =
		{
			c->ViewMatrix(),
			c->ProjectionMatrix(),
			Inverse(c->ViewMatrix()),
			Inverse(c->ProjectionMatrix()),
		};

		mRTHeap->Set();

		mCameraBuffer[mCurrentBackBufferIndex]->Copy(m);

		CreateTopLevelAS(mInstances, this, true);

		UpdateLightsBuffers();

		mRTLightsBuffer[mCurrentBackBufferIndex]->Copy<PerFrameLights, sLight>(mPerFrameLights[mCurrentBackBufferIndex], mObjManager->mLights.size());

		//mCommandList->SetGraphicsRootSignature(mHitSignature.Get());
		//mRTLightsBuffer->Set(2);

		//auto address = mRTLightsBuffer->Resource()->GetGPUVirtualAddress();
		//mCommandList->SetGraphicsRootShaderResourceView(2, address);

		// Transition the RT resources 

		D3D12_RESOURCE_BARRIER barriers[] =
		{
			CD3DX12_RESOURCE_BARRIER::Transition(mOutputResource.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
			CD3DX12_RESOURCE_BARRIER::Transition(mBackBuffers[mCurrentBackBufferIndex]->mResource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST),
			CD3DX12_RESOURCE_BARRIER::Transition(mOutputResource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE),
			CD3DX12_RESOURCE_BARRIER::Transition(mBackBuffers[mCurrentBackBufferIndex]->mResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET),
		};


		mCommandList->ResourceBarrier(1, barriers);


		// Setup the raytracing task
		D3D12_DISPATCH_RAYS_DESC desc = {};
		// The layout of the SBT is as follows: ray generation shader, miss
		// shaders, hit groups. As described in the CreateShaderBindingTable method,
		// all SBT entries of a given type have the same size to allow a fixed stride.
		// The ray generation shaders are always at the beginning of the SBT.
		uint32_t rayGenerationSectionSizeInBytes = mSbtHelper->GetRayGenSectionSize();
		desc.RayGenerationShaderRecord.StartAddress = mSbtStorage->GetGPUVirtualAddress();
		desc.RayGenerationShaderRecord.SizeInBytes = rayGenerationSectionSizeInBytes;

		// The miss shaders are in the second SBT section, right after the ray
		// generation shader. We have one miss shader for the camera rays and one
		// for the shadow rays, so this section has a size of 2*m_sbtEntrySize. We
		// also indicate the stride between the two miss shaders, which is the size
		// of a SBT entry

		uint32_t missSectionSizeInBytes = mSbtHelper->GetMissSectionSize();
		desc.MissShaderTable.StartAddress = mSbtStorage->GetGPUVirtualAddress() + rayGenerationSectionSizeInBytes;
		desc.MissShaderTable.SizeInBytes = missSectionSizeInBytes;
		desc.MissShaderTable.StrideInBytes = mSbtHelper->GetMissEntrySize();

		// The hit groups section start after the miss shaders. In this sample we
		// have one 1 hit group for the triangle
		uint32_t hitGroupsSectionSize = mSbtHelper->GetHitGroupSectionSize();
		desc.HitGroupTable.StartAddress = mSbtStorage->GetGPUVirtualAddress() + rayGenerationSectionSizeInBytes + missSectionSizeInBytes;
		desc.HitGroupTable.SizeInBytes = hitGroupsSectionSize;
		desc.HitGroupTable.StrideInBytes = mSbtHelper->GetHitGroupEntrySize();

		desc.Width = GetScene()->GetViewportX();
		desc.Height = GetScene()->GetViewportY();
		desc.Depth = 1;

		mCommandList->SetPipelineState1(mRaytracingStateObject.Get());

		mCommandList->DispatchRays(&desc);

		mCommandList->ResourceBarrier(2, &barriers[1]);

		mCommandList->CopyResource(mBackBuffers[mCurrentBackBufferIndex]->mResource.Get(), mOutputResource.Get());

		mCommandList->ResourceBarrier(1, &barriers[3]);
	}
}
