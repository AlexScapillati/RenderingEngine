#pragma once

#include "Engine.h"
#include "DX11/DX11Engine.h"
#include "DX12/DX12Engine.h"

// Factory function that creates the correct engine

enum EApiType
{
	EDX11,
	EDX12
};

inline std::unique_ptr<IEngine> NewEngine(EApiType type, HINSTANCE hInstance, int nCmdShow)
{
	try
	{
		switch (type)
		{
		case EDX11:
			return std::make_unique<DX11::CDX11Engine>(hInstance, nCmdShow);
		case EDX12:
			return std::make_unique<DX12::CDX12Engine>(hInstance, nCmdShow);
		default:
			return nullptr;
		}
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(e.what());
	}
}