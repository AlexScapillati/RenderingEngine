#pragma once

#include <atlconv.h>
#include <filesystem>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <d3dcommon.h>
#include <dxgidebug.h>
#include <dxgi1_6.h>
#include "d3dx12.h"

#include <stdexcept>
#include <string>
#include <wrl/client.h>

#include <DirectXMath.h>
#include "..\Math/CVector2.h"
#include "..\Math/CVector3.h"
#include "..\Math/CVector4.h"
#include "..\Math/CMatrix4x4.h"

#include <stdexcept>

#include <imgui.h>
#include <stb_image.h>

#include "../External/DirectXTK12/Inc/CommonStates.h"

namespace DX12Common
{
	using namespace Microsoft::WRL;

	struct CommandAllocatorEntry
	{
		uint64_t fenceValue = 0;
		ComPtr<ID3D12CommandAllocator> commandAllocator;
	};

	using Resource = ComPtr<ID3D12Resource>;


	// Assign a name to the object to aid with debugging.
#if defined(_DEBUG) || defined(DBG)
	inline void SetName(ID3D12Object* pObject, LPCWSTR name)
	{
		pObject->SetName(name);
	}
	inline void SetNameIndexed(ID3D12Object* pObject, LPCWSTR name, UINT index)
	{
		WCHAR fullName[50];
		if (swprintf_s(fullName, L"%s[%u]", name, index) > 0)
		{
			pObject->SetName(fullName);
		}
	}
#else
	inline void SetName(ID3D12Object*, LPCWSTR)
	{
	}
	inline void SetNameIndexed(ID3D12Object*, LPCWSTR, UINT)
	{
	}
#endif

	// Naming helper for ComPtr<T>.
	// Assigns the name of the variable as the name of the object.
	// The indexed variant will include the index in the name of the object.
#define NAME_D3D12_OBJECT(x) SetName((x).Get(), L#x)
#define NAME_D3D12_OBJECT_INDEXED(x, n) SetNameIndexed((x)[n].Get(), L#x, n)


#define ThrowIfFailed(hr)  if(FAILED(hr)) throw std::exception()


	struct SShader
	{
		std::string mFileName;

		ComPtr<ID3DBlob> mblob;

		void LoadShaderFromFile(std::string fileName)
		{
			if (FAILED(D3DReadFileToBlob(ATL::CA2W((fileName + ".cso").c_str()), &mblob)))
			{
				throw std::runtime_error("Error Loading " + fileName);
			}

			mFileName = fileName;
		}
	};


	//--------------------------------------------------------------------------------------
	// Buffers
	//--------------------------------------------------------------------------------------

	struct Vertex
	{
		CVector4 position;
		CVector2 uv;
	};


	struct PerModelConstants
	{
		CMatrix4x4 modelMatrix;
		CMatrix4x4 padding[3];
	};

	constexpr uint64_t MAX_LIGHTS = 64;

	struct SLight
	{
		CVector3 position;
		float    enabled;
		CVector3 colour;
		float    intensity;
	};

	// Data that remains constant for an entire frame, updated from C++ to the GPU shaders *once per frame*
	// We hold them together in a structure and send the whole thing to a "constant buffer" on the GPU each frame when
	// we have finished updating the scene. There is a structure in the shader code that exactly matches this one
	
	struct PerFrameConstants
	{
		// These are the matrices used to position the camera
		CMatrix4x4 cameraMatrix{};
		CMatrix4x4 viewMatrix{};
		CMatrix4x4 projectionMatrix{};
		CMatrix4x4 viewProjectionMatrix{}; // The above two matrices multiplied together to combine their effects
		// 256 bytes 

		CVector3 ambient;
		float specularPower;

		// 16 bytes

		float roughness;
		float metalness;
		bool customValues;
		bool pad[7];

		// 16 bytes

		BYTE padding1[(256  - 32) % 256];

		SLight lights[MAX_LIGHTS]{}; // todo: do a separate buffer

	};
}