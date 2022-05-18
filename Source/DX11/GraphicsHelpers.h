//--------------------------------------------------------------------------------------
// Helper functions to unclutter and simplify main code (Scene.cpp/.h)
//--------------------------------------------------------------------------------------
// Code in .cpp file

#pragma once

#include <atlbase.h> // C-string to unicode conversion function CA2CT
#include <d3d11.h>
#include "DX11Engine.h"

namespace DX11
{


	//--------------------------------------------------------------------------------------
	// Constant buffers
	//--------------------------------------------------------------------------------------

	// Template function to update a constant buffer. Pass the DirectX constant buffer object and the C++ data structure
	// you want to update it with. The structure will be copied in full over to the GPU constant buffer, where it will
	// be available to shaders. This is used to update model and camera positions, lighting data etc.

	inline void CDX11Engine::UpdateModelConstantBuffer(ID3D11Buffer* buffer, DX11::PerModelConstants& bufferData) const
	{
		D3D11_MAPPED_SUBRESOURCE cb;
		mD3DContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);
		memcpy(cb.pData, &bufferData, sizeof(DX11::PerModelConstants));
		mD3DContext->Unmap(buffer, 0);
	}

	inline void CDX11Engine::UpdatePostProcessingConstBuffer(ID3D11Buffer* buffer, DX11::PostProcessingConstants& bufferData) const
	{
		D3D11_MAPPED_SUBRESOURCE cb;
		mD3DContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);
		memcpy(cb.pData, &bufferData, sizeof(DX11::PostProcessingConstants));
		mD3DContext->Unmap(buffer, 0);
	}

	inline void CDX11Engine::UpdateFrameConstantBuffer(ID3D11Buffer* buffer, DX11::PerFrameConstants& bufferData) const
	{
		D3D11_MAPPED_SUBRESOURCE cb;
		mD3DContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);

		memcpy(cb.pData, &bufferData, sizeof(DX11::PerFrameConstants));
		mD3DContext->Unmap(buffer, 0);
	}

	inline void CDX11Engine::UpdateLightConstantBuffer(ID3D11Buffer* buffer, DX11::PerFrameLights& bufferData, int numLights) const
	{
		D3D11_MAPPED_SUBRESOURCE cb;
		mD3DContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);

		//calculate the size of the frame constant buffer

		//remove the lights array from the equation
		auto size = sizeof(DX11::PerFrameLights) - sizeof(bufferData.lights);

		const auto sizeOfLightStruct = sizeof(DX11::sLight);

		//add just the lights used
		size += sizeOfLightStruct * numLights;

		memcpy(cb.pData, &bufferData, size);
		mD3DContext->Unmap(buffer, 0);
	}

	inline void CDX11Engine::UpdateSpotLightsConstantBuffer(ID3D11Buffer* buffer, DX11::PerFrameSpotLights& bufferData, int numLights) const
	{
		D3D11_MAPPED_SUBRESOURCE cb;
		mD3DContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);

		//calculate the size of the frame constant buffer

		//remove the lights array from the equation
		auto size = sizeof(DX11::PerFrameSpotLights) - sizeof(bufferData.spotLights);

		const auto sizeOfLightStruct = sizeof(DX11::sSpotLight);

		//add just the lights used
		size += sizeOfLightStruct * numLights;

		memcpy(cb.pData, &bufferData, size);
		mD3DContext->Unmap(buffer, 0);
	}

	inline void CDX11Engine::UpdateDirLightsConstantBuffer(ID3D11Buffer* buffer, DX11::PerFrameDirLights& bufferData, int numLights) const
	{
		D3D11_MAPPED_SUBRESOURCE cb;
		mD3DContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);

		//calculate the size of the frame constant buffer

		//remove the lights array from the equation
		auto size = sizeof(DX11::PerFrameDirLights) - sizeof(bufferData.dirLights);

		const auto sizeOfLightStruct = sizeof(DX11::sDirLights);

		//add just the lights used
		size += sizeOfLightStruct * numLights;

		memcpy(cb.pData, &bufferData, size);
		mD3DContext->Unmap(buffer, 0);
	}

	inline void CDX11Engine::UpdatePointLightsConstantBuffer(ID3D11Buffer* buffer, DX11::PerFramePointLights& bufferData, int numLights) const
	{
		D3D11_MAPPED_SUBRESOURCE cb;
		mD3DContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);

		//calculate the size of the frame constant buffer

		//remove the lights array from the equation
		auto size = sizeof(DX11::PerFramePointLights) - sizeof(bufferData.pointLights);

		const auto sizeOfLightStruct = sizeof(DX11::sPointLights);

		//add just the lights used
		size += sizeOfLightStruct * numLights;

		memcpy(cb.pData, &bufferData, size);
		mD3DContext->Unmap(buffer, 0);
	}

}