#include "DX12Shader.h"

CDX12Shader::CDX12Shader(CDX12Engine* engine, std::string& absolutePath):
	mEngine(engine),
	mPath(absolutePath)
{
}

CDX12PixelShader::CDX12PixelShader(CDX12Engine* engine, std::string& absolutePath): CDX12Shader(engine, absolutePath)
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

CDX12VertexShader::CDX12VertexShader(CDX12Engine* engine, std::string& absolutePath): CDX12Shader(engine, absolutePath)
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
			nullptr, nullptr, "VSMain", "vs_5_0",
			compileFlags, 0, &mShaderBlob, nullptr));
}
