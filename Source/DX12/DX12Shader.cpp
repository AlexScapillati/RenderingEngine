#include "DX12Shader.h"

namespace DX12
{

	ID3D10Blob* LoadShaderFromFile(const std::string& fileName)
	{
		ID3D10Blob* blob;
		const auto f = (fileName + ".cso");
		if (FAILED(D3DReadFileToBlob(ATL::CA2W(f.c_str()), &blob)))
		{
			throw std::runtime_error("Error Loading " + f);
		}

		return blob;
	}


	CDX12Shader::CDX12Shader(CDX12Engine* engine, const std::string& absolutePath) :
		mEngine(engine),
		mPath(absolutePath)
	{
	}

	CDX12PixelShader::CDX12PixelShader(CDX12Engine* engine, const std::string& absolutePath) : CDX12Shader(engine, absolutePath)
	{

		if (absolutePath.find(".hlsl") != std::string::npos)
		{
#if defined(_DEBUG)
			// Enable better shader debugging with the graphics debugging tools.
			UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
			UINT compileFlags = 0;
#endif

			ThrowIfFailed(
				D3DCompileFromFile(
					std::wstring(absolutePath.begin(), absolutePath.end()).c_str(),
					nullptr, nullptr, "PSMain", "ps_5_0",
					compileFlags, 0, &mShaderBlob, nullptr));
		}
		else
		{
			try
			{
				auto fullPath = mEngine->GetShaderFolder() + absolutePath;
				mShaderBlob = LoadShaderFromFile(fullPath);
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(e.what());
			}
		}
	}

	CDX12VertexShader::CDX12VertexShader(CDX12Engine* engine,const std::string& absolutePath) : CDX12Shader(engine, absolutePath)
	{
		if(absolutePath.find(".hlsl") != std::string::npos)
		{
#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
			UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
			ID3D10Blob* errorBlob;
#else
		UINT compileFlags = 0;
#endif

		ThrowIfFailed(
			D3DCompileFromFile(
				std::wstring(absolutePath.begin(), absolutePath.end()).c_str(),
				nullptr, nullptr, "VSMain", "vs_5_0",
				compileFlags, 0, &mShaderBlob, &errorBlob));

#if defined(_DEBUG)
			if(errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
#endif 
		}
		else
		{
			try
			{
				auto fullPath = mEngine->GetShaderFolder() + absolutePath;
				mShaderBlob = LoadShaderFromFile(fullPath);
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(e.what());
			}
		}
	}
}