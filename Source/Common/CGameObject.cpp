#include "CGameObject.h"

#include <system_error>
#include <filesystem>

#include "../Source/Utility/Input.h"
#include "../Engine.h"
#include "../Math/CVector3.h"
#include "../Math/CMatrix4x4.h"
#include "../Common.h"

void CGameObject::GetFilesInFolder(std::string& mediaFolder, std::string& dirPath, std::vector<std::string>& fileNames) const
{
	//iterate through the directory
	std::filesystem::recursive_directory_iterator iter(dirPath);

	dirPath.replace(0, mediaFolder.size(), "");

	if (dirPath[dirPath.size() - 1] != '/') dirPath.push_back('/');

	std::filesystem::recursive_directory_iterator end;

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

std::string CGameObject::TextureFileName()
{
	return mTextureFiles.front();
}

std::vector<std::string>& CGameObject::GetMeshes() { return mMeshFiles; }

// Control a given node in the model using keys provided. Amount of motion performed depends on frame time
void CGameObject::Control(int     node,
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


// Getting angles from a matrix is complex - see .cpp file
CVector3 CGameObject::Scale(int node)
{
	return {
		Length(mWorldMatrices[node].GetRow(0)),
		Length(mWorldMatrices[node].GetRow(1)),
		Length(mWorldMatrices[node].GetRow(2))
	};
}



void CGameObject::SetRotation(CVector3 rotation, int node)
{
	// To put rotation angles into a matrix we need to build the matrix from scratch to make sure we retain existing scaling and position
	mWorldMatrices[node] = MatrixScaling(Scale(node)) *
		MatrixRotationZ(rotation.z) * MatrixRotationX(rotation.x) * MatrixRotationY(rotation.y) *
		MatrixTranslation(Position(node));
}

// Two ways to set scale: x,y,z separately, or all to the same value
// To set scale without affecting rotation, normalise each row, then multiply it by the scale value.

void CGameObject::SetScale(CVector3 scale, int node)
{
	mWorldMatrices[node].SetRow(0, Normalise(mWorldMatrices[node].GetRow(0)) * scale.x);
	mWorldMatrices[node].SetRow(1, Normalise(mWorldMatrices[node].GetRow(1)) * scale.y);
	mWorldMatrices[node].SetRow(2, Normalise(mWorldMatrices[node].GetRow(2)) * scale.z);
}

// Setters / Getters

void                                  CGameObject::SetVariation(int variation) { if (variation >= 0 && variation < mLODs[mCurrentLOD].size()) LoadNewMesh(mLODs[mCurrentLOD][variation]); }
CVector3                              CGameObject::Position(int node) { return mWorldMatrices[node].GetRow(3); }
CVector3                              CGameObject::Rotation(int node) { return mWorldMatrices[node].GetEulerAngles(); }
void                                  CGameObject::SetPosition(CVector3 position, int node) { mWorldMatrices[node].SetRow(3, position); }
void                                  CGameObject::SetScale(float scale) { SetScale({ scale,scale,scale }); }
void                                  CGameObject::SetWorldMatrix(CMatrix4x4 matrix, int node) { mWorldMatrices[node] = matrix; }
CMatrix4x4&                           CGameObject::WorldMatrix(int node) { return mWorldMatrices[node]; }
float*                                CGameObject::DirectPosition() { return &mWorldMatrices[0].e30; }
std::string                           CGameObject::Name() { return mName; }
void                                  CGameObject::SetName(std::string n) { mName = n; }
float&                                CGameObject::ParallaxDepth() { return mParallaxDepth; }
void                                  CGameObject::SetParallaxDepth(float p) { mParallaxDepth = p; }
bool*                                 CGameObject::Enabled() { return &mEnabled; }
float&                                CGameObject::Roughness() { return mRoughness; }
float&                                CGameObject::Metalness() { return mMetalness; }
void                                  CGameObject::SetRoughness(float r) { mRoughness = r; }
std::vector<std::string>&             CGameObject::GetVariations() { return mLODs[mCurrentLOD]; }
int                                   CGameObject::CurrentLOD() const { return mCurrentLOD; }
std::vector<std::vector<std::string>> CGameObject::LODs() const { return mLODs; }
int                                   CGameObject::CurrentVariation() const { return mCurrentVar; }
void                                  CGameObject::SetLOD(int i) { if (i > 0 && i < mLODs.size()) LoadNewMesh(mLODs[i][mCurrentVar]); }
std::string                           CGameObject::MeshFileNames() { return mMeshFiles.front(); }
bool                                  CGameObject::Update(float updateTime) { return true; } //TODO WIP
