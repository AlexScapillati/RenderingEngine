#include "DX12GameObject.h"

#include <filesystem>
#include <memory>
#include <utility>

#include "../../Utility/HelperFunctions.h"
#include "../../Utility/Input.h"

extern float ROTATION_SPEED;
extern float MOVEMENT_SPEED;

CDX12GameObject::CDX12GameObject(CDX12GameObject& obj)
{
	mEngine    = obj.mEngine;
	mEnabled   = true;
	mMeshFiles = obj.mMeshFiles;
	mMesh      = std::make_unique<CDX12Mesh>(mEngine, obj.mMesh->MeshFileName(), mMesh->Material()->TextureFileNames());
	mName      = "new" + obj.mName;

	mLODs       = obj.mLODs;
	mCurrentLOD = obj.mCurrentLOD;
	mCurrentVar = obj.mCurrentVar;

	mParallaxDepth = obj.ParallaxDepth();
	mRoughness     = obj.Roughness();
	mMetalness     = obj.mMetalness;

	// Set default matrices from mesh
	mWorldMatrices.resize(mMesh->NumberNodes());
	for (auto i = 0; i < mWorldMatrices.size(); ++i) mWorldMatrices[i] = mMesh->GetNodeDefaultMatrix(i);

	SetPosition(obj.Position());
	SetRotation(obj.Rotation());
	SetScale(obj.Scale());
}


CDX12GameObject::CDX12GameObject(CDX12Engine*       engine,
								 const std::string& mesh,
								 const std::string& name,
								 const std::string& diffuseMap,
								 CVector3           position,
								 CVector3           rotation,
								 float              scale)
{
	mEngine        = engine;
	mParallaxDepth = 0.f;
	mRoughness     = 0.5f;
	mMetalness     = 0.0f;

	mEnabled = true;

	if (mesh.empty()) throw std::exception("Error Loading Object");

	mName = std::move(name);

	// Import material
	std::vector maps = { diffuseMap };

	//default model
	//not PBR
	//that could be light models or cube maps
	try
	{
		mMesh = std::make_unique<CDX12Mesh>(mEngine, mesh, maps);
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

void CDX12GameObject::GetFilesInFolder(std::string&              dirPath,
									   std::vector<std::string>& fileNames) const
{
	//iterate through the directory
	std::filesystem::recursive_directory_iterator iter(dirPath);

	dirPath.replace(0, mEngine->GetMediaFolder().size(), "");

	if (dirPath[dirPath.size() - 1] != '/') dirPath.push_back('/');

	const std::filesystem::recursive_directory_iterator end;

	while (iter != end)
	{
		if (!is_directory(iter->path()))
		{
			fileNames.push_back(dirPath + iter->path().filename().string());
			iter.disable_recursion_pending();
		}
		std::error_code ec;
		iter.increment(ec);
		if (ec) { throw std::runtime_error("Error accessing " + ec.message()); }
	}
}

CDX12GameObject::CDX12GameObject(CDX12Engine*       engine,
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
	mRoughness     = 0.5f;
	mMetalness     = 0.0f;

	//search for files with the same id
	std::vector<std::string> files;

	auto folder = engine->GetMediaFolder() + id;

	//	If the id has a dot, this means it has an extention, so we are dealing with a file
	if (id.find_last_of('.') == std::string::npos)
	{
		// Get every file in that folder
		// If the id did not have a dot means that we are dealing with a folder
		GetFilesInFolder(folder, files);
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
			GetFilesInFolder(folder, files);
		}
		else
		{
			// Get the ID
			id = id.substr(0, id.find_first_of('_'));

			GetFilesWithID(engine->GetMediaFolder(), files, id);
		}
	}

	//find meshes trough the files
	for (auto st : files)
	{
		//set the meshes in the vector
		if (st.find(".fbx") != std::string::npos) { mMeshFiles.push_back(st); }
		else if (st.find(".x") != std::string::npos) { mMeshFiles.push_back(st); }
		else if (st.find(".png") != std::string::npos) { mTextureFiles.push_back(st); }
		else if (st.find(".dds") != std::string::npos) { mTextureFiles.push_back(st); }
		else if (st.find(".jpg") != std::string::npos) { mTextureFiles.push_back(st); }
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
		mMesh = std::make_unique<CDX12Mesh>(mEngine, mMeshFiles.front(),mTextureFiles );

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

void CDX12GameObject::Release()
{
}

void CDX12GameObject::Control(int     node,
							  float   frameTime,
							  KeyCode turnUp,
							  KeyCode turnDown,
							  KeyCode turnLeft,
							  KeyCode turnRight,
							  KeyCode turnCW,
							  KeyCode turnCCW,
							  KeyCode moveForward,
							  KeyCode moveBackward)
{
	auto& matrix = mWorldMatrices[node]; // Use reference to node matrix to make code below more readable

	if (KeyHeld(turnUp)) { matrix = MatrixRotationX(ROTATION_SPEED * frameTime) * matrix; }
	if (KeyHeld(turnDown)) { matrix = MatrixRotationX(-ROTATION_SPEED * frameTime) * matrix; }
	if (KeyHeld(turnRight)) { matrix = MatrixRotationY(ROTATION_SPEED * frameTime) * matrix; }
	if (KeyHeld(turnLeft)) { matrix = MatrixRotationY(-ROTATION_SPEED * frameTime) * matrix; }
	if (KeyHeld(turnCW)) { matrix = MatrixRotationZ(ROTATION_SPEED * frameTime) * matrix; }
	if (KeyHeld(turnCCW)) { matrix = MatrixRotationZ(-ROTATION_SPEED * frameTime) * matrix; }

	// Local Z movement - move in the direction of the Z axis, get axis from world matrix
	const auto localZDir = Normalise(matrix.GetRow(2)); // normalise axis in case world matrix has scaling
	if (KeyHeld(moveForward)) { matrix.SetRow(3, matrix.GetRow(3) + localZDir * MOVEMENT_SPEED * frameTime); }
	if (KeyHeld(moveBackward)) { matrix.SetRow(3, matrix.GetRow(3) - localZDir * MOVEMENT_SPEED * frameTime); }
}

CVector3 CDX12GameObject::Scale(int node)
{
	return {
		Length(mWorldMatrices[node].GetRow(0)),
		Length(mWorldMatrices[node].GetRow(1)),
		Length(mWorldMatrices[node].GetRow(2))
	};
}

CVector3       CDX12GameObject::Position(int node) { return mWorldMatrices[node].GetRow(3); }
CVector3       CDX12GameObject::Rotation(int node) { return mWorldMatrices[node].GetEulerAngles(); }
CMatrix4x4&    CDX12GameObject::WorldMatrix(int node) { return mWorldMatrices[node]; }
CDX12Mesh*     CDX12GameObject::Mesh() const { return mMesh.get(); }
std::string    CDX12GameObject::Name() const { return mName; }
void           CDX12GameObject::SetName(std::string n) { mName = std::move(n); }
float&         CDX12GameObject::ParallaxDepth() { return mParallaxDepth; }
void           CDX12GameObject::SetParallaxDepth(float p) { mParallaxDepth = p; }
float*         CDX12GameObject::DirectPosition() { return &mWorldMatrices[0].e30; }
bool*          CDX12GameObject::Enabled() { return &mEnabled; }

std::string CDX12GameObject::TextureFileName() const { return mMesh->Material()->TextureFileNames().front();}

CDX12Material* CDX12GameObject::Material() const { return mMesh->Material(); }
float&         CDX12GameObject::Roughness() { return mRoughness; }
float&         CDX12GameObject::Metalness() { return mMetalness; }
void           CDX12GameObject::SetRoughness(float r) { mRoughness = r; }

void CDX12GameObject::SetPosition(
		CVector3 position,
		int      node) { mWorldMatrices[node].SetRow(3, position); }

void CDX12GameObject::SetScale(float scale) { SetScale({ scale,scale,scale }); }

void CDX12GameObject::SetWorldMatrix(
		CMatrix4x4 matrix,
		int        node) { mWorldMatrices[node] = matrix; }

std::vector<std::string>& CDX12GameObject::GetMeshes() { return mMeshFiles; }
std::vector<std::string>& CDX12GameObject::GetVariations() { return mLODs[mCurrentLOD]; }

void CDX12GameObject::SetVariation(int variation) { if (variation >= 0 && variation < mLODs[mCurrentLOD].size()) LoadNewMesh(mLODs[mCurrentLOD][variation]); }

int  CDX12GameObject::CurrentLOD() const { return mCurrentLOD; }
int  CDX12GameObject::CurrentVariation() const { return mCurrentVar; }
void CDX12GameObject::SetLOD(int i) { if (i > 0 && i < mLODs.size()) LoadNewMesh(mLODs[i][mCurrentVar]); }

std::vector<std::vector<std::string>> CDX12GameObject::LODs() const { return mLODs; }

void CDX12GameObject::SetScale(
		CVector3 scale,
		int      node)
{
	mWorldMatrices[node].SetRow(0, Normalise(mWorldMatrices[node].GetRow(0)) * scale.x);
	mWorldMatrices[node].SetRow(1, Normalise(mWorldMatrices[node].GetRow(1)) * scale.y);
	mWorldMatrices[node].SetRow(2, Normalise(mWorldMatrices[node].GetRow(2)) * scale.z);
}

void CDX12GameObject::SetRotation(
		CVector3 rotation,
		int      node)
{
	// To put rotation angles into a matrix we need to build the matrix from scratch to make sure we retain existing scaling and position
	mWorldMatrices[node] = MatrixScaling(Scale(node)) *
			MatrixRotationZ(rotation.z) * MatrixRotationX(rotation.x) * MatrixRotationY(rotation.y) *
			MatrixTranslation(Position(node));
}

void CDX12GameObject::LoadNewMesh(const std::string& newMesh)
{
	try
	{
		const auto prevPos      = Position();
		const auto prevScale    = Scale();
		const auto prevRotation = Rotation();

		// Recalculate matrix based on mesh
		mWorldMatrices.resize(mMesh->NumberNodes());
		for (auto i = 0; i < mWorldMatrices.size(); ++i) mWorldMatrices[i] = mMesh->GetNodeDefaultMatrix(i);

		SetPosition(prevPos);
		SetScale(prevScale);
		SetRotation(prevRotation);
	}
	catch (const std::exception& e) { throw std::exception(e.what()); }
}

bool CDX12GameObject::Update(float updateTime) { return true; }

void CDX12GameObject::Render()
{
	//if the model is not enable do not render it
	if (!mEnabled) return;

	// Render the mesh
	mMesh->Render(mWorldMatrices);
}
