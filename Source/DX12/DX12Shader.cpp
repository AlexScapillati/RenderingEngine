#include "DX12Shader.h"

#include <filesystem>
#include <functional>
#include <thread_pool.hpp>

#include "DXR/DXR.h"

namespace DX12
{

		int filter(unsigned int code, struct _EXCEPTION_POINTERS* pExceptionInfo) {
			static char scratch[32];
			// report all errors with fputs to prevent any allocation
			if (code == EXCEPTION_ACCESS_VIOLATION) {
				// use pExceptionInfo to document and report error
				fputs("access violation. Attempted to ", stderr);
				if (pExceptionInfo->ExceptionRecord->ExceptionInformation[0])
					fputs("write", stderr);
				else
					fputs("read", stderr);
				fputs(" from address ", stderr);
				sprintf_s(scratch, _countof(scratch), "0x%p\n",
					(void*)pExceptionInfo->ExceptionRecord->ExceptionInformation[1]);
				fputs(scratch, stderr);
				return EXCEPTION_EXECUTE_HANDLER;
			}
			if (code == EXCEPTION_STACK_OVERFLOW) {
				// use pExceptionInfo to document and report error
				fputs("stack overflow\n", stderr);
				return EXCEPTION_EXECUTE_HANDLER;
			}
			fputs("Unrecoverable Error ", stderr);
			sprintf_s(scratch, _countof(scratch), "0x%08x\n", code);
			fputs(scratch, stderr);
			return EXCEPTION_CONTINUE_SEARCH;
		}

		HRESULT Compile(IDxcCompiler3 * pCompiler, DxcBuffer * pSource, LPCWSTR pszArgs[],
			int argCt, IDxcIncludeHandler * pIncludeHandler, IDxcResult * *pResults) {

			__try {
				return pCompiler->Compile(
					pSource,                // Source buffer.
					pszArgs,                // Array of pointers to arguments.
					argCt,                  // Number of arguments.
					pIncludeHandler,        // User-provided interface to handle #include directives (optional).
					IID_PPV_ARGS(pResults) // Compiler output status, buffer, and errors.
				);
			}
			__except (filter(GetExceptionCode(), GetExceptionInformation())) {
				// UNRECOVERABLE ERROR!
				// At this point, state could be extremely corrupt. Terminate the process
				return E_FAIL;
			}
		}

	IDxcBlob* CompileShader(LPCWSTR fileName, std::vector<LPCWSTR>& args)
	{


		ComPtr<IDxcUtils> pUtils;
		ComPtr<IDxcCompiler3>	pCompiler;
		ComPtr<IDxcLibrary>		pLibrary;
		ComPtr<IDxcIncludeHandler> dxcIncludeHandler;

		ThrowIfFailed(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtils.GetAddressOf())));
		ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(pCompiler.ReleaseAndGetAddressOf())));
		ThrowIfFailed(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(pLibrary.ReleaseAndGetAddressOf())));

		ThrowIfFailed(pUtils->CreateDefaultIncludeHandler(dxcIncludeHandler.ReleaseAndGetAddressOf()));

		std::wstring absolutePath = std::filesystem::current_path().wstring() + L"/Source/Shaders/";

		auto file = std::wstring(fileName);

		if (std::wstring(fileName).find(absolutePath) == std::wstring::npos)
		{
			file = absolutePath + file;
		}


		// Open and read the file
		std::ifstream shaderFile(file);
		if (shaderFile.good() == false)
		{
			throw std::logic_error("Cannot find shader file");
		}

		auto f = std::wstring(file);
		std::wstring justTheName;
		auto pos = f.find_last_of('/');
		std::wstring folder;
		if (pos != std::wstring::npos)
		{
			folder = L"-I " + f.substr(0, pos);
			args.push_back(folder.c_str());

			// Get only the name of the shader (used for storing the PDB)

			justTheName = f.substr(pos);
			auto p = justTheName.find_first_of('.');
			if(p)
			{
				justTheName = justTheName.substr(0, p);
			}
		}


		std::stringstream strStream;
		strStream << shaderFile.rdbuf();
		std::string sShader = strStream.str();

		ComPtr<IDxcBlobEncoding> pSource;
		pUtils->CreateBlob(sShader.c_str(), static_cast<uint32_t>(sShader.size()), CP_UTF8, pSource.GetAddressOf());

		DxcBuffer sourceBuffer;
		sourceBuffer.Ptr = pSource->GetBufferPointer();
		sourceBuffer.Size = pSource->GetBufferSize();
		sourceBuffer.Encoding = 0;

		auto pbdArg =  std::wstring(L"-Fd") + absolutePath;
			pbdArg = pbdArg.append(justTheName) + L".pbd";

		//args.push_back(L"-Qstrip_debug");
		args.push_back(pbdArg.c_str());
		args.push_back(DXC_ARG_DEBUG); //-Zi
		args.push_back(DXC_ARG_WARNINGS_ARE_ERRORS); //-WX
		

		// Compile
		ComPtr<IDxcResult> pResult;
		try
		{
			auto hr = Compile(pCompiler.Get(), &sourceBuffer, args.data(), static_cast<uint32_t>(args.size()), dxcIncludeHandler.Get(), pResult.GetAddressOf());
			ThrowIfFailed(hr);
		}
		catch(std::exception& e)
		{
			throw std::exception(e.what());
		}

		//
		// Print errors if present.
		//
		ComPtr<IDxcBlobUtf8> pErrors = nullptr;
		// Note that d3dcompiler would return null if no errors or warnings are present.
		// IDxcCompiler3::Compile will always return an error buffer,
		// but its length will be zero if there are no warnings or errors.
		if (SUCCEEDED(pResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr)) && pErrors != nullptr && pErrors->GetStringLength() != 0)
			OutputDebugStringA(pErrors->GetStringPointer());

		//
		// Quit if the compilation failed.
		//
		HRESULT hrStatus;
		if (FAILED(pResult->GetStatus(&hrStatus)) || FAILED(hrStatus))
		{
			// Compilation failed, but successful HRESULT was returned.
			// Could reuse the compiler and allocator objects. For simplicity, exit here anyway
			return nullptr;
		}


		//
		// Save pdb.
		//
		ComPtr<IDxcBlob> pPDB = nullptr;
		ComPtr<IDxcBlobUtf16> pPDBName = nullptr;
			auto hr = pResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(pPDB.GetAddressOf()), pPDBName.GetAddressOf());
		if (SUCCEEDED(hr))
		{
			FILE* fp = NULL;

			// Note that if you don't specify -Fd, a pdb name will be automatically generated.
			// Use this file name to save the pdb so that PIX can find it quickly.
			_wfopen_s(&fp, pPDBName->GetStringPointer(), L"wb");
			fwrite(pPDB->GetBufferPointer(), pPDB->GetBufferSize(), 1, fp);
			fclose(fp);
		}

		IDxcBlob* pBlob;
		ThrowIfFailed(pResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pBlob), nullptr));
		return pBlob;
	}



	CDX12Shader::CDX12Shader(CDX12Engine* engine, const std::string& absolutePath) :
		mEngine(engine),
		mPath(absolutePath)
	{
	}

	CDX12PixelShader::CDX12PixelShader(CDX12Engine* engine, const std::string& absolutePath) : CDX12Shader(engine, absolutePath)
	{

		auto path = absolutePath;
		LPCWSTR entry = L"-EPSMain";
		LPCWSTR target = L"-Tps_6_1";
		
		if (absolutePath.find(".hlsl") == std::string::npos)
		{
			if (auto pos2 = absolutePath.find(".cso") != std::string::npos)
				path.erase(pos2, 3);
			path.append(".hlsl");
			entry = L"-Emain";
		}
		try
		{
			std::vector args{ entry, target };
			
			auto file = std::wstring(path.begin(), path.end());

			auto task = std::function(CompileShader);

			auto l = [](auto p, auto a) {return CompileShader(p, a); };

			thread_pool ThreadPool;

			auto future = ThreadPool.submit(l, file.c_str(), args);

			ThreadPool.wait_for_tasks();

			mShaderBlob = future.get();

			//mShaderBlob = CompileShader(file.c_str(),args);
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error(e.what());
		}
	}

	CDX12VertexShader::CDX12VertexShader(CDX12Engine* engine,const std::string& absolutePath) : CDX12Shader(engine, absolutePath)
	{

		auto path = absolutePath;
		LPCWSTR entry = L"-EVSMain";
		LPCWSTR target = L"-Tvs_6_1";

		if (absolutePath.find(".hlsl") == std::string::npos)
		{
			if (auto pos2 = absolutePath.find(".cso") != std::string::npos)
				path.erase(pos2, 3);
			path.append(".hlsl");
			entry = L"-Emain";
		}
		try
		{

			std::vector args{ entry, target };

			auto file = std::wstring(path.begin(), path.end());

			mShaderBlob = CompileShader(file.c_str(), args);;
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error(e.what());
		}
	}
}
