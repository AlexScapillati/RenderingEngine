
#pragma once

#include "../DX12Common.h"
#include "../DX12Engine.h"

#include "dxcapi.h"

#include <fstream>
#include <sstream>

namespace DX12
{
	namespace  DXR
	{

		//--------------------------------------------------------------------------------------------------
		//
		//
		inline ID3D12Resource* CreateBuffer(ID3D12Device* m_device, uint64_t size,
			D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState,
			const D3D12_HEAP_PROPERTIES& heapProps)
		{
			D3D12_RESOURCE_DESC bufDesc = {};
			bufDesc.Alignment = 0;
			bufDesc.DepthOrArraySize = 1;
			bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			bufDesc.Flags = flags;
			bufDesc.Format = DXGI_FORMAT_UNKNOWN;
			bufDesc.Height = 1;
			bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			bufDesc.MipLevels = 1;
			bufDesc.SampleDesc.Count = 1;
			bufDesc.SampleDesc.Quality = 0;
			bufDesc.Width = size;

			ID3D12Resource* pBuffer;
			ThrowIfFailed(m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc,
				initState, nullptr, IID_PPV_ARGS(&pBuffer)));
			return pBuffer;
		}

#ifndef ROUND_UP
#define ROUND_UP(v, powerOf2Alignment) (((v) + (powerOf2Alignment)-1) & ~((powerOf2Alignment)-1))
#endif

		// Specifies a heap used for uploading. This heap type has CPU access optimized
		// for uploading to the GPU.
		static const D3D12_HEAP_PROPERTIES kUploadHeapProps = {
			D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0, 0 };

		// Specifies the default heap. This heap type experiences the most bandwidth for
		// the GPU, but cannot provide CPU access.
		static const D3D12_HEAP_PROPERTIES kDefaultHeapProps = {
			D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0, 0 };


	}


	ComPtr<ID3D12RootSignature> CreateRayGenSignature(ID3D12Device* device);

	ComPtr<ID3D12RootSignature>CreateMissSignature(ID3D12Device* device);

	ComPtr<ID3D12RootSignature> CreateHitSignature(ID3D12Device* device);

	/// Create the acceleration structure of an instance
	///
	/// \param vVertexBuffers : pair of buffer and vertex count
	///	\param vIndexBuffers : pair of buffer and index count
	/// \return AccelerationStructureBuffers for TLAS
	AccelerationStructureBuffers CreateBottomLevelAS(
		CDX12Engine * engine,
		std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vVertexBuffers,
		uint32_t vertexSize,
		std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vIndexBuffers
		);

	/// Create the main acceleration structure that holds
	/// all instances of the scene
	/// \param instances : pair of BLAS and transform
	void CreateTopLevelAS(std::vector<std::pair<ComPtr<ID3D12Resource>, CMatrix4x4*>>& instances, CDX12Engine* engine, bool updateOnly = false);

	/// Create all acceleration structures, bottom and top
	void CreateAccelerationStructures(CDX12Engine* engine);
	
}
