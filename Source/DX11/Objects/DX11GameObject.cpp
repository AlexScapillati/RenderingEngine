//--------------------------------------------------------------------------------------
// Class encapsulating a model
//--------------------------------------------------------------------------------------
// Holds a pointer to a mesh as well as position, rotation and scaling, which are converted to a world matrix when required
// This is more of a convenience class, the Mesh class does most of the difficult work.

#include "DX11GameObject.h"

#include "../DX11Engine.h"
#include "../DX11Scene.h"
#include "../GraphicsHelpers.h"
#include "../../Utility/HelperFunctions.h"
#include "../../Common/CGameObjectManager.h"


namespace DX11
{
	//copy constructor
	CDX11GameObject::CDX11GameObject(CDX11GameObject& obj)
	{
		mEngine = obj.mEngine;

		//initialize ambient map variables
		mAmbientMap.size    = obj.AmbientMap()->size;
		mAmbientMap.enabled = obj.AmbientMap()->enabled;
		mAmbientMap.Init(mEngine);
	}

	CDX11GameObject::CDX11GameObject(CDX11Engine* engine, const std::string& mesh, const std::string& name, const std::string& diffuseMap, CVector3 position, CVector3 rotation , float scale)
	{
		mEngine             = engine;
		mAmbientMap.enabled = false;
		mAmbientMap.size    = 4;
		mAmbientMap.Init(mEngine);

		mParallaxDepth = 0.f;
		mRoughness     = 0.5f;
		mMetalness     = 0.0f;

		mEnabled = true;

		if (mesh.empty()) throw std::exception("Error Loading Object");

		mName = name;

		// Import material
		try
		{
			std::vector maps = { diffuseMap };
			mMaterial        = std::make_unique<CDX11Material>(maps, mEngine);
		}
		catch (const std::exception& e) { throw std::runtime_error(e.what()); }

		//default model
		//not PBR
		//that could be light models or cube maps
		try
		{
			mMesh = std::make_unique<CDX11Mesh>(mEngine, mesh, mMaterial->HasNormals());
			mMeshFiles.push_back(mesh);

			// Set default matrices from mesh
			mWorldMatrices.resize(mMesh->NumberNodes());
			for (auto i = 0; i < mWorldMatrices.size(); ++i) mWorldMatrices[i] = mMesh->GetNodeDefaultMatrix(i);
		}
		catch (const std::exception& e) { throw std::runtime_error(e.what()); }

		//geometry loaded, set its position...

		SetPosition(position);
		CGameObject::SetRotation(rotation);
		SetScale(scale);
	}


	CDX11GameObject::CDX11GameObject(CDX11Engine* engine,
									 const std::string& dirPath, std::string name, CVector3 position /*= { 0,0,0 }*/, CVector3 rotation /*= { 0,0,0 }*/, float scale /*= 1*/): CGameObject()
	{
		mEngine = engine;

		//initialize member variables
		mName = std::move(name);

		//initialize ambient map variables
		mAmbientMap.size    = 4;
		mAmbientMap.enabled = false;
		mAmbientMap.Init(mEngine);

		mEnabled = true;

		mParallaxDepth = 0.f;
		mRoughness     = 0.5f;
		mMetalness     = 0.0f;

		//search for files with the same id
		std::vector<std::string> files;

		std::string id = dirPath;

		auto folder = engine->GetMediaFolder() + dirPath;

		//	If the id has a dot, this means it has an extention, so we are dealing with a file
		if (id.find_last_of('.') == std::string::npos)
		{
			// Get every file in that folder
			// If the id did not have a dot means that we are dealing with a folder
			GetFilesInFolder(mEngine, folder, files);
		}
		else
		{
			// Get the position of the last underscore
			auto nPos = id.find_last_of('_');

			//Get folder
			auto slashPos = id.find_last_of('/');

			if (slashPos != std::string::npos)
			{
				// Get the id
				auto subFolder = id.substr(0, slashPos);

				folder = engine->GetMediaFolder() + subFolder + '/';

				//get every file that is beginning with that id
				GetFilesInFolder(mEngine, folder, files);
			}
			else
			{
				// Get the ID
				id = id.substr(0, id.find_first_of('_'));

				GetFilesWithID(engine->GetMediaFolder(), files, id);
			}
		}

		//create the material
		mMaterial = std::make_unique<CDX11Material>(files, mEngine);

		//find meshes trough the files
		for (auto st : files)
		{
			//set the meshes in the vector
			if (st.find(".fbx") != std::string::npos) { mMeshFiles.push_back(st); }
			else if (st.find(".x") != std::string::npos) { mMeshFiles.push_back(st); }
		}

		if (mMeshFiles.empty()) { throw std::runtime_error("No mesh found in " + name); }

		// If the meshes vector has more than one fileName
		if (mMeshFiles.size() > 1)
		{
			// Extract the lods and variations
			int                      counter     = 0;
			int                      nVariations = 0;
			std::vector<std::string> vec;
			for (auto mesh : mMeshFiles)
			{
				// Prepare file to find
				// It changes depending on the counter
				std::string currFind = "LOD";
				currFind += (counter + '0');

				// If found
				if (mesh.find(currFind) != std::string::npos)
				{
					// Push it in the variations vector
					vec.push_back(mesh);
				}
				else
				{
					// If not found we finished the variations for this LOD
					// So push the variations vector in the LODs vector and clear the variations vector
					mLODs.push_back(move(vec));

					// Push the current variation iin the variation vec
					vec.push_back(mesh);

					// Increment the LOD counter
					counter++;
				}
			}
		}
		else
		{
			// If the meshes vector has only one fileName
			// Push it on the lod
			mLODs.push_back(mMeshFiles);
		}

		try
		{
			//load the most detailed mesh with tangents required if the model has normals
			mMesh = std::make_unique<CDX11Mesh>(mEngine, mMeshFiles.front(), mMaterial->HasNormals());

			// Set default matrices from mesh
			mWorldMatrices.resize(mMesh->NumberNodes());
			for (auto i = 0; i < mWorldMatrices.size(); ++i) mWorldMatrices[i] = mMesh->GetNodeDefaultMatrix(i);
		}
		catch (std::exception& e) { throw std::runtime_error(e.what()); }

		// geometry loaded, set its position...
		SetPosition(position);
		SetRotation(rotation);
		SetScale(scale);
	}

	// The render function simply passes this model's matrices over to Mesh:Render.
	// All other per-frame constants must have been set already along with shaders, textures, samplers, states etc.
	void CDX11GameObject::Render(bool basicGeometry)
	{
		//if the model is not enable do not render it
		if (!mEnabled) return;

		// Set the parallax depth to the constant buffer
		gPerModelConstants.parallaxDepth = mParallaxDepth;

		// Set the material roughness to the constant buffer
		gPerModelConstants.roughness = mRoughness;
		gPerModelConstants.metalness = mMetalness;

		if (!basicGeometry)
		{
			if (mAmbientMap.enabled) { mEngine->GetContext()->PSSetShaderResources(6, 1, mAmbientMap.mapSRV.GetAddressOf()); }
			/*else if (dynamic_cast<CSky*>(GOM->GetSky())->HasCubeMap());
		{
			auto environmentMap = GOM->GetSky()->TextureSRV();
			gD3DContext->PSSetShaderResources(6, 1, &environmentMap);
		}*/
		}

		// Render the material
		mMaterial->RenderMaterial(basicGeometry);

		// Render the mesh
		mMesh->Render(mWorldMatrices);

		// Unbind the ambient map from the shader
		ID3D11ShaderResourceView* nullView = nullptr;
		mEngine->GetContext()->PSSetShaderResources(6, 1, &nullView);
	}

	void CDX11GameObject::RenderToAmbientMap()
	{
		// if the ambient map is disabled, nothing to do here
		if (!mAmbientMap.enabled) return;

		// Store current RS state
		ID3D11RasterizerState* prevRS = nullptr;
		mEngine->GetContext()->RSGetState(&prevRS);

		// Set the cullback state
		mEngine->GetContext()->RSSetState(mEngine->mCullBackState.Get());

		// Store current RTV(render target) and DSV(depth stencil)
		ID3D11RenderTargetView* prevRTV = nullptr;
		ID3D11DepthStencilView* prevDSV = nullptr;

		mEngine->GetContext()->OMGetRenderTargets(1, &prevRTV, &prevDSV);

		//create the viewport
		D3D11_VIEWPORT vp;
		vp.Width    = static_cast<FLOAT>(mAmbientMap.size);
		vp.Height   = static_cast<FLOAT>(mAmbientMap.size);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		mEngine->GetContext()->RSSetViewports(1, &vp);

		float mSides[6][3] = {
			// Starting from facing down the +ve Z direction, left handed rotations
			{0.0f, 0.5f, 0.0f},  // +ve X direction (values multiplied by PI)
			{0.0f, -0.5f, 0.0f}, // -ve X direction
			{-0.5f, 0.0f, 0.0f}, // +ve Y direction
			{0.5f, 0.0f, 0.0f},  // -ve Y direction
			{0.0f, 0.0f, 0.0f},  // +ve Z direction
			{0.0f, 1.0f, 0.0f}   // -ve Z direction
		};

		const auto& originalRotation = Rotation();

		// for all six faces of the cubemap
		for (int i = 0; i < 6; ++i)
		{
			// change rotation
			SetRotation(CVector3(mSides[i]) * PI);

			mEngine->GetContext()->ClearDepthStencilView(mAmbientMap.depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
			mEngine->GetContext()->OMSetRenderTargets(1, mAmbientMap.RTV[i].GetAddressOf(), mAmbientMap.depthStencilView.Get());

			// Update Frame buffer
			gPerFrameConstants.viewMatrix           = InverseAffine(WorldMatrix());
			gPerFrameConstants.projectionMatrix     = MakeProjectionMatrix(1.0f, ToRadians(90.0f));
			gPerFrameConstants.viewProjectionMatrix = gPerFrameConstants.viewMatrix * gPerFrameConstants.projectionMatrix;

			// Update Constant buffer
			mEngine->UpdateFrameConstantBuffer(gPerFrameConstantBuffer.Get(), gPerFrameConstants);

			// Set it to the GPU
			mEngine->GetContext()->PSSetConstantBuffers(1, 1, gPerFrameConstantBuffer.GetAddressOf());
			mEngine->GetContext()->VSSetConstantBuffers(1, 1, gPerFrameConstantBuffer.GetAddressOf());

			mEngine->GetObjManager()->mSky->Render();

			//render just the objects that can cast shadows
			for (const auto& it : mEngine->GetObjManager()->mObjects)
			{
				// Do not render this object
				if (it != this)
					// Render all the objects 
					// Performance improvements: Could render only the closest objects
					// Could render only the face that the user is looking at (kinda like cull back state)
					it->Render();
			}
		}

		//restore render target, otherwise the ambient map will not be sent to the shader because it is still bound as a render target
		mEngine->GetContext()->OMSetRenderTargets(1, &prevRTV, prevDSV);

		if (prevRTV) prevRTV->Release();
		if (prevDSV) prevDSV->Release();

		// Restore previous RS state
		mEngine->GetContext()->RSSetState(prevRS);

		// Release that
		if (prevRS) prevRS->Release();

		//restore original rotation
		SetRotation(originalRotation);

		//generate mipMaps for the cube map
		mEngine->GetContext()->GenerateMips(mAmbientMap.mapSRV.Get());
	}

	void CDX11GameObject::sAmbientMap::Init(CDX11Engine* engine)
	{
		mEngine = engine;

		//http://richardssoftware.net/Home/Post/26

		//initialize the texture map cube
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width                = size;
		textureDesc.Height               = size;
		textureDesc.MipLevels            = 0;
		textureDesc.ArraySize            = 6; //6 faces
		textureDesc.Format               = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count     = 1;
		textureDesc.SampleDesc.Quality   = 0;
		textureDesc.Usage                = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags            = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET; //use it to passit to the shader and use it as a render target
		textureDesc.CPUAccessFlags       = 0;
		textureDesc.MiscFlags            = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;

		//create it
		auto hr = mEngine->GetDevice()->CreateTexture2D(&textureDesc, nullptr, map.GetAddressOf());
		if (FAILED(hr)) { throw std::runtime_error("Error creating cube texture"); }

		//create render target views
		D3D11_RENDER_TARGET_VIEW_DESC viewDesc  = {};
		viewDesc.Format                         = textureDesc.Format;
		viewDesc.ViewDimension                  = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		viewDesc.Texture2DArray.FirstArraySlice = 0;
		viewDesc.Texture2DArray.ArraySize       = 1;
		viewDesc.Texture2DArray.MipSlice        = 0;

		//create 6 of them
		for (int i = 0; i < 6; ++i)
		{
			viewDesc.Texture2DArray.FirstArraySlice = i;
			hr                                      = mEngine->GetDevice()->CreateRenderTargetView(map.Get(), &viewDesc, RTV[i].GetAddressOf());
			if (FAILED(hr)) { throw std::runtime_error("Error creating render target view"); }
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format                          = textureDesc.Format;
		srvDesc.ViewDimension                   = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MostDetailedMip     = 0;
		srvDesc.TextureCube.MipLevels           = -1;

		hr = mEngine->GetDevice()->CreateShaderResourceView(map.Get(), &srvDesc, mapSRV.GetAddressOf());
		if (FAILED(hr)) { throw std::runtime_error("Error creating cube map SRV"); }

		//now can release the texture
		map->Release();

		//create depth stencil
		D3D11_TEXTURE2D_DESC dsDesc = {};
		dsDesc.Width                = size;
		dsDesc.Height               = size;
		dsDesc.MipLevels            = 1;
		dsDesc.ArraySize            = 1;
		dsDesc.Format               = DXGI_FORMAT_R32_TYPELESS;
		dsDesc.SampleDesc.Count     = 1;
		dsDesc.SampleDesc.Quality   = 0;
		dsDesc.Usage                = D3D11_USAGE_DEFAULT;
		dsDesc.BindFlags            = D3D11_BIND_DEPTH_STENCIL;
		dsDesc.CPUAccessFlags       = 0;
		dsDesc.MiscFlags            = 0;

		//create it
		if (FAILED(mEngine->GetDevice()->CreateTexture2D(&dsDesc, NULL, depthStencilMap.GetAddressOf()))) { throw std::runtime_error("Error creating depth stencil"); }

		//create depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format                        = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.Flags                         = 0;
		dsvDesc.ViewDimension                 = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice            = 0;

		if (FAILED(mEngine->GetDevice()->CreateDepthStencilView(depthStencilMap.Get(), &dsvDesc, depthStencilView.GetAddressOf()))) { throw std::runtime_error("Error creating depth stencil view "); }

		//release the depth stencil texture since we are done with it
		depthStencilMap->Release();
	}


	void CDX11GameObject::LoadNewMesh(std::string newMesh)
	{
		try
		{
			mMesh = std::make_unique<CDX11Mesh>(mEngine, newMesh, mMaterial->HasNormals());

			auto prevPos      = Position();
			auto prevScale    = Scale();
			auto prevRotation = Rotation();

			// Recalculate matrix based on mesh
			mWorldMatrices.resize(mMesh->NumberNodes());
			for (auto i = 0; i < mWorldMatrices.size(); ++i) mWorldMatrices[i] = mMesh->GetNodeDefaultMatrix(i);

			SetPosition(prevPos);
			SetScale(prevScale);
			SetRotation(prevRotation);
		}
		catch (const std::exception& e) { throw std::exception(e.what()); }
	}

	ID3D11ShaderResourceView*     CDX11GameObject::TextureSRV() const { return mMaterial->TextureSRV(); }
	CDX11GameObject::sAmbientMap* CDX11GameObject::AmbientMap() { return &mAmbientMap; }
	ID3D11ShaderResourceView*     CDX11GameObject::AmbientMapSRV() const { return mAmbientMap.mapSRV.Get(); }
	UINT                          CDX11GameObject::sAmbientMap::Size() const { return size; }
	CDX11Mesh*                    CDX11GameObject::Mesh() const { return mMesh.get(); }
	CDX11Material*                CDX11GameObject::Material() const { return mMaterial.get(); }


	bool& CDX11GameObject::AmbientMapEnabled() { return mAmbientMap.enabled; }

	void CDX11GameObject::sAmbientMap::SetSize(UINT s)
	{
		// Set the size
		size = s;

		// Re initialize all the maps to the new size
		Init(mEngine);
	}




}
