

//--------------------------------------------------------------------------------------
// Class encapsulating a mesh
//--------------------------------------------------------------------------------------
// The mesh class splits the mesh into sub-meshes that only use one texture each.

#pragma once

#include "DX12Common.h"

#include "DX12ConstantBuffer.h"

struct aiNode;

namespace DX12
{
	class CDX12Engine;
	class CDX12Material;
	class CDX12PBRPSO;

	class CDX12Mesh
	{
		//--------------------------------------------------------------------------------------
		// Private data structures
		//--------------------------------------------------------------------------------------

		// A mesh is made of multiple sub-meshes. Each one uses a single material (texture).
		// Each sub-mesh has a vertex / index buffer on the GPU. Could share buffers for performance but that would be complex.
		struct SubMesh
		{
			uint32_t       vertexSize = 0;         // Size in bytes of a single vertex (depends on what it contains, uvs, tangents etc.)

			//// GPU-side vertex and index buffers
			uint32_t       numVertices = 0;

			uint32_t                         numIndices;
			ComPtr<ID3D12Resource>           mIndexBuffer;
			D3D12_INDEX_BUFFER_VIEW          indexBufferView;
			std::unique_ptr<unsigned char[]> indices;

			ComPtr<ID3D12Resource> mVertexBuffer;
			D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
			std::unique_ptr<unsigned char[]> vertices;
		};


		// A mesh contains a hierarchy of nodes. A node represents a seperate animatable part of the mesh
		// A node can contain several sub-meshes (because a single node might use multiple textures)
		// A node can also have child nodes. The children will follow the motion of the parent node
		// Each node has a default matrix which is it's initial/ default position. Models using this mesh are
		// given these default matrices as a starting position.
		struct Node
		{
			std::string  name;
			CMatrix4x4   defaultMatrix; // Starting position/rotation/scale for this node. Relative to parent. Used when first creating a model from this mesh
			CMatrix4x4   offsetMatrix;

			unsigned int parentIndex;   // Index of the parent node (from the mNodes vector below). Root node refers to itself (0)

			std::vector<unsigned int> childNodes; // Child nodes that are controlled by this node (indexes into the mNodes vector below)
			std::vector<unsigned int> subMeshes;  // The geometry representing this node (indexes into the mSubMeshes vector below)
		};


		//--------------------------------------------------------------------------------------
		// Construction / Usage
		//--------------------------------------------------------------------------------------
	public:

		CDX12Mesh() = delete;
		CDX12Mesh(const CDX12Mesh&&) = delete;
		CDX12Mesh& operator=(const CDX12Mesh&) = delete;
		CDX12Mesh& operator=(const CDX12Mesh&&) = delete;

		// Pass the name of the mesh file to load. Uses assimp (http://www.assimp.org/) to support many file types
		// Optionally request tangents to be calculated (for normal and parallax mapping - see later lab)
		// Will throw a std::runtime_error exception on failure (since constructors can't return errors).
		CDX12Mesh(CDX12Engine* engine, std::string fileName, bool requireTangents = false);

		CDX12Mesh(const CDX12Mesh&);

		// How many nodes are in the hierarchy for this mesh. Nodes can control individual parts (rigid body animation),
		// or bones (skinned animation), or they can be dummy nodes to create child parts in a more convenient way
		unsigned int NumberNodes() const { return static_cast<unsigned int>(mNodes.size()); }

		// The default matrix for a given node - used to set the initial position for a new model
		CMatrix4x4 GetNodeDefaultMatrix(unsigned int node) const { return mNodes[node].defaultMatrix; }

		// Render the mesh with the given matrices
		// Handles rigid body meshes (including single part meshes) as well as skinned meshes
		// LIMITATION: The mesh must use a single texture throughout
		void Render(std::vector<CMatrix4x4>& modelMatrices);

		std::string MeshFileName() const { return mFileName; }

		auto& ModelConstants() { return mModelConstants; }

		//--------------------------------------------------------------------------------------
		// Private helper functions
		//--------------------------------------------------------------------------------------
	private:

		// Count the number of nodes with given assimp node as root
		unsigned int CountNodes(aiNode* assimpNode);

		// Help build the arrays of submeshes and nodes from the assimp data - recursive
		unsigned int ReadNodes(aiNode* assimpNode, unsigned int nodeIndex, unsigned int parentIndex);

		// Helper function for Render function - renders a given sub-mesh. World matrices / textures / states etc. must already be set
		void RenderSubMesh(const SubMesh& subMesh) const;

		//--------------------------------------------------------------------------------------
		// Member data
		//--------------------------------------------------------------------------------------

		CDX12Engine* mEngine;

		public:


		std::string mFileName;			//store the filename for the copy constructor
		bool hasTangents;			//store if the mesh has tangents, same reason for above

		std::vector<SubMesh> mSubMeshes; // The mesh geometry. Nodes refer to sub-meshes in this vector
		std::vector<Node>    mNodes;     // The mesh hierarchy. First entry is root. remainder aree stored in depth-first order

		bool mHasBones; // If any submesh has bones, then all submeshes are given bones - makes rendering easier (one shader for the whole mesh)

		std::unique_ptr<CDX12ConstantBuffer> mModelConstantBuffer;

		PerModelConstants  mModelConstants;
	};
}
