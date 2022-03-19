#include "DX12GameObject.h"

#include "../../Utility/HelperFunctions.h"

#include "../DX12Engine.h"

#include "../DX12PipelineObject.h"

namespace DX12
{
	CDX12GameObject::CDX12GameObject(CDX12GameObject& obj)
		:CDX12GameObject(obj.mEngine, obj.MeshFileNames(), obj.Name(), obj.Position(), obj.Rotation(), obj.Scale().x)
	{
	}


	CDX12GameObject::CDX12GameObject(CDX12Engine* engine,
		const std::string& mesh,
		const std::string& name,
		const std::string& diffuseMap,
		CVector3           position,
		CVector3           rotation,
		float              scale)
	{
		mEngine = engine;
		mParallaxDepth = 0.f;
		mRoughness = 0.5f;
		mMetalness = 0.0f;

		mEnabled = true;

		if (mesh.empty()) throw std::exception("Error Loading Object");

		mName = name;

		// Import material
		mTextureFiles = { diffuseMap };

		//default model
		//not PBR
		//that could be light models or cube maps
		try
		{

			mMaterial = std::make_unique<CDX12Material>(mTextureFiles, mEngine);

			mMesh = std::make_unique<CDX12Mesh>(mEngine, mesh);
			mMeshFiles.push_back(mesh);

			// Set default matrices from mesh
			mWorldMatrices.resize(mMesh->NumberNodes());
			for (auto i = 0; i < mWorldMatrices.size(); ++i) mWorldMatrices[i] = mMesh->GetNodeDefaultMatrix(i);
		}
		catch (const std::exception& e) { throw std::runtime_error(e.what()); }

		//geometry loaded, set its position...
		SetPosition(position);
		SetRotation(rotation);
		SetScale(scale);
	}


	CDX12GameObject::CDX12GameObject(CDX12Engine* engine,
		std::string        id,
		const std::string& name,
		CVector3           position,
		CVector3           rotation,
		float              scale)
	{
		mEngine = engine;

		//initialize member variables
		mName = name;

		mEnabled = true;

		mParallaxDepth = 0.f;
		mRoughness = 0.5f;
		mMetalness = 0.0f;

		//search for files with the same id
		std::vector<std::string> files;

		auto folder = engine->GetMediaFolder() + id;

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

		//find meshes trough the files
		for (const auto& st : files)
		{
			//set the meshes in the vector
			if (st.find(".fbx") != std::string::npos) mMeshFiles.push_back(st);
			else if (st.find(".x") != std::string::npos)  mMeshFiles.push_back(st);
			else if (st.find(".png") != std::string::npos) mTextureFiles.push_back(st);
			else if (st.find(".dds") != std::string::npos) mTextureFiles.push_back(st);
			else if (st.find(".jpg") != std::string::npos) mTextureFiles.push_back(st);
		}

		if (mMeshFiles.empty()) { throw std::runtime_error("No mesh found in " + name); }

		// If the meshes vector has more than one fileName
		if (mMeshFiles.size() > 1)
		{
			// Extract the lods and variations
			int                      counter = 0;
			int                      nVariations = 0;
			std::vector<std::string> vec;
			for (const auto& mesh : mMeshFiles)
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
			mMesh = std::make_unique<CDX12Mesh>(mEngine, mMeshFiles.front(), true);

			mMaterial = std::make_unique<CDX12Material>(mTextureFiles, mEngine);

			// Set default matrices from mesh
			mWorldMatrices.resize(mMesh->NumberNodes());
			for (auto i = 0; i < mWorldMatrices.size(); ++i) mWorldMatrices[i] = mMesh->GetNodeDefaultMatrix(i);
		}
		catch (std::exception& e) { throw std::runtime_error(e.what()); }

		// geometry loaded, set its position...
		SetPosition(position);
		CGameObject::SetRotation(rotation);
		SetScale(scale);
	}

	CDX12Material* CDX12GameObject::Material() const { return mMaterial.get(); }

	void CDX12GameObject::LoadNewMesh(std::string newMesh)
	{
		try
		{

			mMesh = nullptr;

			const auto prevPos = Position();
			const auto prevScale = Scale();
			const auto prevRotation = Rotation();

			mMesh = std::make_unique<CDX12Mesh>(mEngine,newMesh,IsPbr());

			// Recalculate matrix based on mesh
			mWorldMatrices.resize(mMesh->NumberNodes());
			for (auto i = 0; i < mWorldMatrices.size(); ++i) mWorldMatrices[i] = mMesh->GetNodeDefaultMatrix(i);

			SetPosition(prevPos);
			SetScale(prevScale);
			SetRotation(prevRotation);
		}
		catch (const std::exception& e) { throw std::exception(e.what()); }
	}

	void CDX12GameObject::Render(bool basicGeometry)
	{
		//if the model is not enable do not render it
		if (!mEnabled) return;

			// Set the pipeline state object
		if(!basicGeometry) mEngine->mPbrPso->Set(mEngine->mCommandList.Get());

		// Render the material
		mMaterial->RenderMaterial();

		mEngine->SetConstantBuffers();

		// Render the mesh
		mMesh->Render(mWorldMatrices);
	}

	void CDX12Plant::LoadNewMesh(std::string newMesh) { CDX12GameObject::LoadNewMesh(newMesh); }
	void CDX12Plant::Render(bool basicGeometry) { CDX12GameObject::Render(basicGeometry); }
}
