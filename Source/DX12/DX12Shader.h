#pragma once

#include "DX12Engine.h"

namespace DX12
{


	IDxcBlob* CompileShader(LPCWSTR fileName, std::vector<LPCWSTR>& args);


	class CDX12Shader
	{
	public:

		CDX12Shader(CDX12Engine* engine, const std::string& absolutePath);

		CDX12Engine* mEngine;
		std::string mPath;
		ComPtr<IDxcBlob> mShaderBlob;
	};

	class CDX12PixelShader : public CDX12Shader
	{
	public:
		CDX12PixelShader(CDX12Engine* engine, const std::string& absolutePath);
	};


	class CDX12VertexShader : public CDX12Shader
	{
	public:
		CDX12VertexShader(CDX12Engine* engine,const  std::string& absolutePath);
	};
}