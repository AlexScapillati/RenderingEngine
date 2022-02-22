#include "DX12Mesh.h"

#include "DX12Common.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/DefaultLogger.hpp>

#include "DX12DescriptorHeap.h"
#include "DX12ConstantBuffer.h"

#include "CDX12Material.h"
#include "DX12PipelineObject.h"

namespace DX12
{

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
			vertexElements.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, positionOffset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			offset += 12;

			if (!assimpMesh->HasNormals())
				throw std::runtime_error("No normal data for sub-mesh " + subMeshName + " in " + fileName);
			auto normalOffset = offset;
			vertexElements.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, normalOffset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
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
			//
			// DirectX Stuff
			//
			//-----------------------------------

			// Create pipeline state object
			mPbrPipelineStateObject = std::make_unique<CDX12PBRPSO>(mEngine, vertexElements, mEngine->vs.get(), mEngine->ps.get());

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

		// Create the constant buffer.
		{
			constexpr UINT constantBufferSize = sizeof(PerModelConstants); // CB size is required to be 256-byte aligned.

			mModelConstantBuffer = std::make_unique<CDX12ConstantBuffer>(mEngine, constantBufferSize);

			mModelConstantBuffer->Copy(mModelConstants);
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

		mPbrPipelineStateObject->Set(commandList);


		// Render the material
		mMaterial->RenderMaterial();

		// Render a mesh without skinning. Although slightly reorganised to use the matrices calculated
		// above, this is basically the same code as the rigid body animation lab
		// Iterate through each node
		for (unsigned int nodeIndex = 0; nodeIndex < mNodes.size(); ++nodeIndex)
		{
			// Send this node's matrix to the GPU via a constant buffer
			mModelConstants.modelMatrix = absoluteMatrices[nodeIndex];

			// Set the frame constant buffer heap and set the correct root parameter to pass to the vertex shader
			{
				mEngine->mCBVDescriptorHeap->Set();
				mEngine->mPerFrameConstantBuffer->Set(1);
				mEngine->mPerFrameLightsConstantBuffer->Set(2);
			}

			// Set the model constant buffer heap and set the correct root parameter to pass to the vertex shader
			{
				mModelConstantBuffer->Set(0);
				mModelConstantBuffer->Copy(mModelConstants);
			}
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

}