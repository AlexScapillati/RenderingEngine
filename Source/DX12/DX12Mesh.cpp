#include "DX12Mesh.h"

#include "CDX12Common.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/DefaultLogger.hpp>

#include "DX12DescriptorHeap.h"
#include "DX12ConstantBuffer.h"

#include "CDX12Material.h"

CDX12Mesh::CDX12Mesh(const CDX12Mesh& other) : CDX12Mesh(other.mEngine, other.mFileName, other.Material()->TextureFileNames()) {}


CDX12Mesh::CDX12Mesh(CDX12Engine* engine, std::string fileName, std::vector<std::string>& tex)
{
	mEngine = engine;

	fileName = mEngine->GetMediaFolder() + fileName;

	mFileName = fileName;

	hasTangents = true;

	Assimp::Importer importer;

	// Flags for processing the mesh. Assimp provides a huge amount of control - right click any of these
	// and "Peek Definition" to see documention above each constant
	unsigned int assimpFlags = aiProcess_MakeLeftHanded |
		aiProcess_GenSmoothNormals |
		aiProcess_FixInfacingNormals |
		aiProcess_GenUVCoords |
		aiProcess_TransformUVCoords |
		aiProcess_FlipUVs |
		aiProcess_FlipWindingOrder |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_ImproveCacheLocality |
		aiProcess_SortByPType |
		aiProcess_FindInvalidData |
		aiProcess_OptimizeMeshes |
		aiProcess_FindInstances |
		aiProcess_FindDegenerates |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_Debone |
		aiProcess_SplitByBoneCount |
		aiProcess_LimitBoneWeights |
		aiProcess_RemoveComponent;

	// Flags to specify what mesh data to ignore
	auto removeComponents = aiComponent_LIGHTS | aiComponent_CAMERAS | aiComponent_COLORS |
		aiComponent_ANIMATIONS;

	// Add / remove tangents as required by user
	if (hasTangents)
	{
		assimpFlags |= aiProcess_CalcTangentSpace;
	}
	else
	{
		removeComponents |= aiComponent_TANGENTS_AND_BITANGENTS;
	}

	// Other miscellaneous settings
	importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f); // Smoothing angle for normals
	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);  // Remove points and lines (keep triangles only)
	importer.SetPropertyBool(AI_CONFIG_PP_FD_REMOVE, true);                 // Remove degenerate triangles
	importer.SetPropertyBool(AI_CONFIG_PP_DB_ALL_OR_NONE, true);            // Default to removing bones/weights from meshes that don't need skinning

	// Set maximum bones that can affect one vertex, and also maximum bones affecting a single mesh
	unsigned int maxBonesPerVertex = 4; // The shaders support 4 bones per verted (null bones are added if necessary)
	unsigned int maxBonesPerMesh = 256; // Bone indexes are stored in a byte, so no more than 256
	importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, maxBonesPerVertex);
	importer.SetPropertyInteger(AI_CONFIG_PP_SBBC_MAX_BONES, maxBonesPerMesh);

	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, removeComponents);

	// Import mesh with assimp given above requirements - log output
	Assimp::DefaultLogger::create("", Assimp::DefaultLogger::VERBOSE);
	auto scene = importer.ReadFile(fileName, assimpFlags);
	Assimp::DefaultLogger::kill();
	if (scene == nullptr)  throw std::runtime_error("Error loading mesh (" + fileName + "). " + importer.GetErrorString());
	if (scene->mNumMeshes == 0)  throw std::runtime_error("No usable geometry in mesh: " + fileName);

	//-----------------------------------

	//*********************************************************************//
	// Read node hierachy - each node has a matrix and contains sub-meshes //

	// Uses recursive helper functions to build node hierarchy
	mNodes.resize(CountNodes(scene->mRootNode));
	ReadNodes(scene->mRootNode, 0, 0);

	//******************************************//
	// Read geometry - multiple parts supported //

	mHasBones = false;
	for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
		if (scene->mMeshes[m]->HasBones())  mHasBones = true;

	// A mesh is made of sub-meshes, each one can have a different material (texture)
	// Import each sub-mesh in the file to seperate index / vertex buffer (could share buffers between sub-meshes but that would make things more complex)
	mSubMeshes.resize(scene->mNumMeshes);
	for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
	{
		auto assimpMesh = scene->mMeshes[m];
		std::string subMeshName = assimpMesh->mName.C_Str();
		auto& subMesh = mSubMeshes[m]; // Short name for the submesh we're currently preparing - makes code below more readable

		//-----------------------------------

		// Check for presence of position and normal data. Tangents and UVs are optional.
		std::vector<D3D12_INPUT_ELEMENT_DESC> vertexElements;
		unsigned int offset = 0;

		if (!assimpMesh->HasPositions())
			throw std::runtime_error("No position data for sub-mesh " + subMeshName + " in " + fileName);
		auto positionOffset = offset;
		vertexElements.push_back({ "position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, positionOffset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		offset += 12;

		if (!assimpMesh->HasNormals())
			throw std::runtime_error("No normal data for sub-mesh " + subMeshName + " in " + fileName);
		auto normalOffset = offset;
		vertexElements.push_back({ "normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, normalOffset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		offset += 12;

		auto tangentOffset = offset;
		if (hasTangents)
		{
			if (!assimpMesh->HasTangentsAndBitangents())  throw std::runtime_error("No tangent data for sub-mesh " + subMeshName + " in " + fileName);
			vertexElements.push_back({ "tangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, tangentOffset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			offset += 12;
		}

		auto uvOffset = offset;
		if (assimpMesh->GetNumUVChannels() > 0 && assimpMesh->HasTextureCoords(0))
		{
			if (assimpMesh->mNumUVComponents[0] != 2)  throw std::runtime_error("Unsupported texture coordinates in " + subMeshName + " in " + fileName);
			vertexElements.push_back({ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, uvOffset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			offset += 8;
		}

		subMesh.vertexSize = offset;


		//-----------------------------------

		// Create CPU-side buffers to hold current mesh data - exact content is flexible so can't use a structure for a vertex - so just a block of bytes
		// Note: for large arrays a unique_ptr is better than a vector because vectors default-initialise all the values which is a waste of time.
		subMesh.numVertices = assimpMesh->mNumVertices;
		subMesh.numIndices = assimpMesh->mNumFaces * 3;
		subMesh.vertices = std::make_unique<unsigned char[]>(subMesh.numVertices * subMesh.vertexSize);
		subMesh.indices = std::make_unique<unsigned char[]>(subMesh.numIndices * 4); // Using 32 bit indexes (4 bytes) for each indeex

		//-----------------------------------

		// Copy mesh data from assimp to our CPU-side vertex buffer

		auto assimpPosition = reinterpret_cast<CVector3*>(assimpMesh->mVertices);
		auto position = subMesh.vertices.get() + positionOffset;
		auto positionEnd = position + subMesh.numVertices * subMesh.vertexSize;
		while (position != positionEnd)
		{
			*(CVector3*)position = *assimpPosition;
			position += subMesh.vertexSize;
			++assimpPosition;
		}

		auto assimpNormal = reinterpret_cast<CVector3*>(assimpMesh->mNormals);
		auto normal = subMesh.vertices.get() + normalOffset;
		auto normalEnd = normal + subMesh.numVertices * subMesh.vertexSize;
		while (normal != normalEnd)
		{
			*(CVector3*)normal = *assimpNormal;
			normal += subMesh.vertexSize;
			++assimpNormal;
		}

		if (hasTangents)
		{
			auto assimpTangent = reinterpret_cast<CVector3*>(assimpMesh->mTangents);
			auto tangent = subMesh.vertices.get() + tangentOffset;
			auto tangentEnd = tangent + subMesh.numVertices * subMesh.vertexSize;
			while (tangent != tangentEnd)
			{
				*(CVector3*)tangent = *assimpTangent;
				tangent += subMesh.vertexSize;
				++assimpTangent;
			}
		}

		if (assimpMesh->GetNumUVChannels() > 0 && assimpMesh->HasTextureCoords(0))
		{
			auto assimpUV = assimpMesh->mTextureCoords[0];
			auto uv = subMesh.vertices.get() + uvOffset;
			auto uvEnd = uv + subMesh.numVertices * subMesh.vertexSize;
			while (uv != uvEnd)
			{
				*(CVector2*)uv = CVector2(assimpUV->x, assimpUV->y);
				uv += subMesh.vertexSize;
				++assimpUV;
			}
		}

		//-----------------------------------

		// Copy face data from assimp to our CPU-side index buffer
		if (!assimpMesh->HasFaces())  throw std::runtime_error("No face data in " + subMeshName + " in " + fileName);

		auto index = reinterpret_cast<DWORD*>(subMesh.indices.get());
		for (unsigned int face = 0; face < assimpMesh->mNumFaces; ++face)
		{
			*index++ = assimpMesh->mFaces[face].mIndices[0];
			*index++ = assimpMesh->mFaces[face].mIndices[1];
			*index++ = assimpMesh->mFaces[face].mIndices[2];
		}

		//-----------------------------------
		 // Create a root signature.
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if (FAILED(engine->mDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		// Allow input layout and deny unnecessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		// constant root parameters that are used by the vertex shader.
		CD3DX12_DESCRIPTOR_RANGE1 ranges[9] = {};
		CD3DX12_ROOT_PARAMETER1 rootParameters[9] = {};

		ranges[ModelCB].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // per model constant buffer
		ranges[FrameCB].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1); // per frame constant buffer
		ranges[LightsCB].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2); // per lights constant buffer

		ranges[Albedo]		.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,1, 0); // texture
		ranges[Roughness]	.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,1, 1); // texture
		ranges[AO]			.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,1, 2); // texture
		ranges[Displacement].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,1, 3); // texture
		ranges[Normal]		.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,1, 4); // texture
		ranges[Metalness]	.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,1, 5); // texture
		

		rootParameters[ModelCB].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[FrameCB].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[LightsCB].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_ALL);


		rootParameters[Albedo]		.InitAsDescriptorTable(1, &ranges[3], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[Roughness]	.InitAsDescriptorTable(1, &ranges[4], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[AO]			.InitAsDescriptorTable(1, &ranges[5], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[Displacement].InitAsDescriptorTable(1, &ranges[6], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[Normal]		.InitAsDescriptorTable(1, &ranges[7], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[Metalness]	.InitAsDescriptorTable(1, &ranges[8], D3D12_SHADER_VISIBILITY_PIXEL);

		auto samplerDesc = DirectX::CommonStates::StaticAnisotropicWrap(0);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
		rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 1u, &samplerDesc, rootSignatureFlags);
		// TODO: when using point clamp filtering, the texture looks weird

		// Serialize the root signature.
		ComPtr<ID3DBlob> rootSignatureBlob;
		ComPtr<ID3DBlob> errorBlob;
		D3DX12SerializeVersionedRootSignature(&rootSignatureDescription, featureData.HighestVersion, &rootSignatureBlob, &errorBlob);

		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}

		// Create the root signature.
		ThrowIfFailed(engine->mDevice->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
			rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));

		NAME_D3D12_OBJECT(mRootSignature);

		// Create pipeline state
		{

			// Get the shaders
			ID3DBlob* vertexShader;
			ID3DBlob* pixelShader;

#if defined(_DEBUG)
			// Enable better shader debugging with the graphics debugging tools.
			UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
			UINT compileFlags = 0;
#endif

			std::string absolutePath = std::filesystem::current_path().string() + "/Source/Shaders/SimpleShader.hlsl";

			ThrowIfFailed(D3DCompileFromFile(std::wstring(absolutePath.begin(), absolutePath.end()).c_str(),
				nullptr, nullptr, "VSMain", "vs_5_0",
				compileFlags, 0, &vertexShader, nullptr));

			ThrowIfFailed(D3DCompileFromFile(std::wstring(absolutePath.begin(), absolutePath.end()).c_str(),
				nullptr, nullptr, "PSMain", "ps_5_0",
				compileFlags, 0, &pixelShader, nullptr));

			// Define the vertex input layout.
			D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
				{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
					D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
					D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24,
					D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{"UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36,
					D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0} };

			// Describe and create the graphics pipeline state object (PSO).
			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
			psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
			psoDesc.pRootSignature = mRootSignature.Get();
			psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader);
			psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader);
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

		auto hProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto buffer = CD3DX12_RESOURCE_DESC::Buffer(subMesh.vertexSize * subMesh.numVertices);

		ThrowIfFailed(engine->mDevice->CreateCommittedResource(
			&hProp,
			D3D12_HEAP_FLAG_NONE,
			&buffer,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&subMesh.mVertexBuffer)));
		NAME_D3D12_OBJECT(subMesh.mVertexBuffer);

		// Copy the data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.

		ThrowIfFailed(subMesh.mVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, subMesh.vertices.get(), subMesh.numVertices * subMesh.vertexSize);
		subMesh.mVertexBuffer->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		subMesh.mVertexBufferView.BufferLocation = subMesh.mVertexBuffer->GetGPUVirtualAddress();
		subMesh.mVertexBufferView.StrideInBytes = subMesh.vertexSize;
		subMesh.mVertexBufferView.SizeInBytes = subMesh.vertexSize * subMesh.numVertices;

		// Index Buffer
		buffer = CD3DX12_RESOURCE_DESC::Buffer(subMesh.numIndices * sizeof(uint32_t));

		ThrowIfFailed(engine->mDevice->CreateCommittedResource(
			&hProp,
			D3D12_HEAP_FLAG_NONE,
			&buffer,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&subMesh.IndexBuffer)));
		NAME_D3D12_OBJECT(subMesh.IndexBuffer);

		// Copy the data to the index buffer.
		UINT8* pIndexDataBegin;
		ThrowIfFailed(subMesh.IndexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
		memcpy(pIndexDataBegin, subMesh.indices.get(), subMesh.numIndices * sizeof(uint32_t));
		subMesh.IndexBuffer->Unmap(0, nullptr);

		// Initialize the index buffer view.
		subMesh.indexBufferView.BufferLocation = subMesh.IndexBuffer->GetGPUVirtualAddress();
		subMesh.indexBufferView.SizeInBytes = sizeof(uint32_t) * subMesh.numIndices;
		subMesh.indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	}

	// Describe and create a constant buffer view (CBV) descriptor heap.
	{
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
		cbvHeapDesc.NumDescriptors = mEngine->mNumFrames;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(mEngine->mDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mModelCBVDescriptorHeap)));
		NAME_D3D12_OBJECT(mModelCBVDescriptorHeap);
	}

	// Create the constant buffer.
	{
		constexpr UINT constantBufferSize = sizeof(DX12Common::PerModelConstants); // CB size is required to be 256-byte aligned.

		auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

		auto buffer = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);

		ThrowIfFailed(mEngine->mDevice->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&buffer,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&mModelConstantBuffer)));

		// Describe and create a constant buffer view.
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = mModelConstantBuffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = constantBufferSize;
		mEngine->mDevice->CreateConstantBufferView(&cbvDesc, mModelCBVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		// Map and initialize the constant buffer. We don't unmap this until the
		// app closes. Keeping things mapped for the lifetime of the resource is okay.
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		ThrowIfFailed(mModelConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mModelCbvDataBegin)));
		memcpy(mModelCbvDataBegin, &mModelConstants, sizeof(mModelConstants));
		mModelConstantBuffer->Unmap(0, nullptr);
	}

	if (scene->HasTextures())
	{
		std::vector<std::string> textures(scene->mNumTextures);

		for (auto i = 0u; i < scene->mNumTextures; ++i)
		{
			textures.push_back(scene->mTextures[i]->mFilename.C_Str());
		}

		mMaterial = std::make_unique<CDX12Material>(textures, mEngine);
	}
	else
	{
		mMaterial = std::make_unique<CDX12Material>(tex, mEngine);
	}
}

void CDX12Mesh::Render(std::vector<CMatrix4x4>& modelMatrices)
{
	// Skinning needs all matrices available in the shader at the same time, so first calculate all the absolute
	// matrices before rendering anything
	std::vector<CMatrix4x4> absoluteMatrices(modelMatrices.size());
	absoluteMatrices[0] = modelMatrices[0]; // First matrix for a model is the root matrix, already in world space
	for (unsigned int nodeIndex = 1; nodeIndex < mNodes.size(); ++nodeIndex)
	{
		// Multiply each model matrix by its parent's absolute world matrix (already calculated earlier in this loop)
		// Same process as for rigid bodies, simply done prior to rendering now
		absoluteMatrices[nodeIndex] = modelMatrices[nodeIndex] * absoluteMatrices[mNodes[nodeIndex].parentIndex];
	}

	const auto commandList = mEngine->mCommandList.Get();

	commandList->SetGraphicsRootSignature(mRootSignature.Get());
	commandList->SetPipelineState(mPipelineState.Get());

	// Render a mesh without skinning. Although slightly reorganised to use the matrices calculated
	// above, this is basically the same code as the rigid body animation lab
	// Iterate through each node
	for (unsigned int nodeIndex = 0; nodeIndex < mNodes.size(); ++nodeIndex)
	{
		// Send this node's matrix to the GPU via a constant buffer
		mModelConstants.modelMatrix = absoluteMatrices[nodeIndex];

		ID3D12DescriptorHeap* ppHeaps[1] = {};

		// Set the frame constant buffer heap and set the correct root parameter to pass to the vertex shader
		// TODO: move it to scene.cpp
		{
			mEngine->mCBVDescriptorHeap->Set();
			mEngine->mPerFrameConstantBuffer->Set(FrameCB);
			mEngine->mPerFrameLightsConstantBuffer->Set(LightsCB);
		}

		// TODO: move to gameobject.cpp

		// Set the model constant buffer heap and set the correct root parameter to pass to the vertex shader
		{
			ppHeaps[0] = mModelCBVDescriptorHeap.Get();
			commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
			commandList->SetGraphicsRootDescriptorTable(ModelCB, mModelCBVDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		}

		// TODO: move to gameobject.cpp

		// Copy model constant buffer
		{
			CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
			// copy memory in the model vertex buffer
			ThrowIfFailed(mModelConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mModelCbvDataBegin)));
			memcpy(mModelCbvDataBegin, &mModelConstants, sizeof(mModelConstants));
			mModelConstantBuffer->Unmap(0, nullptr);
		}

		// Render the material
		mMaterial->RenderMaterial();

		// Render the sub-meshes attached to this node (no bones - rigid movement)
		for (const auto& subMeshIndex : mNodes[nodeIndex].subMeshes)
		{
			RenderSubMesh(mSubMeshes[subMeshIndex]);
		}
	}
}

unsigned CDX12Mesh::CountNodes(aiNode* assimpNode)
{
	unsigned int count = 1;
	for (unsigned int child = 0; child < assimpNode->mNumChildren; ++child)
		count += CountNodes(assimpNode->mChildren[child]);
	return count;
}

unsigned int CDX12Mesh::ReadNodes(aiNode* assimpNode, unsigned int nodeIndex, unsigned int parentIndex)
{
	auto& node = mNodes[nodeIndex];
	node.parentIndex = parentIndex;
	const auto thisIndex = nodeIndex;
	++nodeIndex;

	node.name = assimpNode->mName.C_Str();

	node.defaultMatrix.SetValues(&assimpNode->mTransformation.a1);
	node.defaultMatrix.Transpose(); // Assimp stores matrices differently to this app

	node.subMeshes.resize(assimpNode->mNumMeshes);
	for (unsigned int i = 0; i < assimpNode->mNumMeshes; ++i)
	{
		node.subMeshes[i] = assimpNode->mMeshes[i];
	}

	node.childNodes.resize(assimpNode->mNumChildren);
	for (unsigned int i = 0; i < assimpNode->mNumChildren; ++i)
	{
		node.childNodes[i] = nodeIndex;
		nodeIndex = ReadNodes(assimpNode->mChildren[i], nodeIndex, thisIndex);
	}

	return nodeIndex;
}

void CDX12Mesh::RenderSubMesh(const SubMesh& subMesh) const
{
	// Set vertex buffer as next data source for GPU
	mEngine->mCommandList->IASetVertexBuffers(0, 1, &subMesh.mVertexBufferView);

	// Set index buffer as next data source for GPU, indicate it uses 32-bit integers
	mEngine->mCommandList->IASetIndexBuffer(&subMesh.indexBufferView);

	// Using triangle lists only in this class
	mEngine->mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Render mesh
	mEngine->mCommandList->DrawIndexedInstanced(subMesh.numIndices, 1, 0, 0, 0);
}

