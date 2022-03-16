#pragma once

#include <atlconv.h>
#include <filesystem>
#include <chrono>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <d3dcommon.h>
#include <dxgidebug.h>
#include <dxgi1_6.h>
#include "d3dx12.h"

#include "pix3.h"

#include <stdexcept>
#include <string>
#include <wrl/client.h>

#include "..\Math/CVector2.h"
#include "..\Math/CVector3.h"
#include "..\Math/CVector4.h"
#include "..\Math/CMatrix4x4.h"

#include <stdexcept>

#include "../External/DirectXTK12/Inc/CommonStates.h"


// The min/max macros conflict with like-named member functions.
// Only use std::min and std::max defined in <algorithm>.

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

namespace DX12
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

	

	struct SHandle
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE mCpu;
		CD3DX12_GPU_DESCRIPTOR_HANDLE mGpu;
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
		CMatrix4x4 worldMatrix;

		CVector3 objectColour;  // Allows each light model to be tinted to match the light colour they cast
		float    parallaxDepth; // Used in the geometry shader to control how much the polygons are exploded outwards

		float hasOpacityMap;
		float hasAoMap;
		float hasRoughnessMap;
		float hasAmbientMap;
		float hasMetallnessMap;

		float roughness;
		float metalness;

		float padding[37];
	};

	constexpr auto s = sizeof(PerModelConstants);

	constexpr uint64_t MAX_LIGHTS = 64;


	// Data that remains constant for an entire frame, updated from C++ to the GPU shaders *once per frame*
	// We hold them together in a structure and send the whole thing to a "constant buffer" on the GPU each frame when
	// we have finished updating the scene. There is a structure in the shader code that exactly matches this one
	
	struct PerFrameConstants
	{

		// These are the matrices used to position the camera
		CMatrix4x4 cameraMatrix;
		CMatrix4x4 viewMatrix;
		CMatrix4x4 projectionMatrix;
		CMatrix4x4 viewProjectionMatrix; // The above two matrices multiplied together to combine their effects

		CVector3 ambientColour;
		float    specularPower;

		float parallaxMinSamples = 5;
		float parallaxMaxSamples = 20;
		float parallaxPadding;

		float gDepthAdjust = 0.00005f;

		float nLights;
		float nDirLight;
		float nSpotLights;
		float nPointLights;

		uint32_t nPcfSamples;
		CVector3 padding2;

		CVector3 cameraPosition;
		float    frameTime; // This app does updates on the GPU so we pass over the frame update time

		float padding[44];
	};


	//--------------------------------------------------------------------------------------
	// Light Structures
	//--------------------------------------------------------------------------------------

	struct sLight
	{
		CVector3 position;
		float    enabled;
		CVector3 colour;
		float    intensity;
	};

	struct sSpotLight
	{
		CVector3   colour;
		float      enabled;
		CVector3   pos;
		float      intensity;
		CVector3   facing;       //the direction facing of the light
		float      cosHalfAngle; //pre calculate this in the c++ side, for performance reasons
		CMatrix4x4 viewMatrix;   //the light view matrix (as it was a camera)
		CMatrix4x4 projMatrix;   //--"--
	};

	struct sDirLight
	{
		CVector3   colour;
		float      enabled;
		CVector3   facing;
		float      intensity;
		CMatrix4x4 viewMatrix; //the light view matrix (as it was a camera)
		CMatrix4x4 projMatrix; //--"--
	};

	struct sPointLight
	{
		CVector3   colour;
		float      enabled;
		CVector3   position;
		float      intensity;
		CMatrix4x4 viewMatrices[6]; //the light view matrix (as it was a camera)
		CMatrix4x4 projMatrix;      //--"--
	};

	struct PerFrameLights
	{
		sLight lights[MAX_LIGHTS];
	};

	struct PerFrameSpotLights
	{
		sSpotLight spotLights[MAX_LIGHTS];
	};

	struct PerFrameDirLights
	{
		sDirLight dirLights[MAX_LIGHTS];
	};

	struct PerFramePointLights
	{
		sPointLight pointLights[MAX_LIGHTS];
	};
	
}