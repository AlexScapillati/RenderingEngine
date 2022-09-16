//--------------------------------------------------------------------------------------
// Helper functions to unclutter and simplify code elsewhere
//--------------------------------------------------------------------------------------

#include "GraphicsHelpers.h"
#include "DDSTextureLoader\DDSTextureLoader11.h"
#include "WICTextureLoader\WICTextureLoader11.h"
#include "DirectXTex.h"
#include "ScreenGrab.h"

namespace DX11
{


	//--------------------------------------------------------------------------------------
	// Texture Loading
	//--------------------------------------------------------------------------------------

	// Using Microsoft's open source DirectX Tool Kit (DirectXTK) to simplify texture loading
	// This function requires you to pass a ID3D11Resource* (e.g. &gTilesDiffuseMap), which manages the GPU memory for the
	// texture and also a ID3D11ShaderResourceView* (e.g. &gTilesDiffuseMapSRV), which allows us to use the texture in shaders
	// The function will fill in these pointers with usable data. Returns false on failure
	bool CDX11Engine::LoadTexture(std::string filename, ID3D11Resource** texture, ID3D11ShaderResourceView** textureSRV)
	{

		HRESULT res = false;

		filename = mMediaFolder + filename;

		std::unique_lock l(mMutex);

		// DDS files need a different function from other files
		std::string dds = ".dds"; // So check the filename extension (case insensitive)
		if (filename.size() >= 4 &&
			std::equal(dds.rbegin(), dds.rend(), filename.rbegin(), [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); }))
		{
			res = DirectX::CreateDDSTextureFromFile(mD3DDevice.Get(), std::wstring(filename.begin(), filename.end()).c_str(), texture, textureSRV);
		}
		else
		{
			res = DirectX::CreateWICTextureFromFile(mD3DDevice.Get(), mD3DContext.Get(), std::wstring(filename.begin(), filename.end()).c_str(), texture, textureSRV);
		}

		return SUCCEEDED(res);
	}

	CVector3 GetTextureDimentions(ID3D11Resource* texture)
	{
		ID3D11Texture2D* tex = nullptr;
		if (SUCCEEDED(texture->QueryInterface(&tex)))
		{
			D3D11_TEXTURE2D_DESC desc;
			tex->GetDesc(&desc);

			CVector3 dim;

			dim.x = static_cast<float>(desc.Width);
			dim.y = static_cast<float>(desc.Height);

			tex->Release();

			return dim;
		}

		return nullptr;
	}



	bool CDX11Engine::SaveTextureToFile(ID3D11Resource* tex, std::string& fileName)
	{
		return 1;
	}
}