//--------------------------------------------------------------------------------------
// Class encapsulating a camera
//--------------------------------------------------------------------------------------
// Holds position, rotation, near/far clip and field of view. These to a view and projection matrices as required

#pragma once

#include "..\Math\CVector3.h"
#include "..\Math\CVector2.h"
#include "..\Math\CMatrix4x4.h"
#include "..\Math/MathHelpers.h"
#include "..\Utility/Input.h"

class CCamera
{
public:

	CCamera(const CCamera&) = delete;
	CCamera(const CCamera&&) = delete;
	CCamera& operator=(const CCamera&) = delete;
	CCamera& operator=(const CCamera&&) = delete;


	//-------------------------------------
	// Construction and Usage
	//-------------------------------------

	// Constructor - initialise all settings, sensible defaults provided for everything.
	CCamera(CVector3 position = {0,0,0}, CVector3 rotation = {0,0,0}, 
           float fov = PI/3, float aspectRatio = 16.0f / 9.0f, float nearClip = 0.1f, float farClip = 10000.0f);

	~CCamera(){};


	// Control the camera's position and rotation using keys provided
	void Control( float frameTime, KeyCode turnUp, KeyCode turnDown, KeyCode turnLeft, KeyCode turnRight,  
	              KeyCode moveForward, KeyCode moveBackward, KeyCode moveLeft, KeyCode moveRight);

	void ControlMouse(float frameTime, CVector2 delta, KeyCode moveForward, KeyCode moveBackward, KeyCode moveLeft, KeyCode moveRight);

	void SetAspectRatio(float r);


	//-------------------------------------
	// Data access
	//-------------------------------------

	// Getters / setters
	CVector3 Position() const;
	CVector3 Rotation() const;
	void     SetPosition(CVector3 position);
	void     SetRotation(CVector3 rotation);

	float FOV() const;
	float NearClip() const;
	float FarClip() const;

	void SetFOV     (float fov     );
	void SetNearClip(float nearClip);
	void SetFarClip (float farClip );

	// Read only access to camera matrices, updated on request from position, rotation and camera settings
	CMatrix4x4& WorldMatrix();
	CMatrix4x4& ViewMatrix();
	CMatrix4x4& ProjectionMatrix();
	CMatrix4x4& ViewProjectionMatrix();

	//-------------------------------------
	// Camera picking
	//-------------------------------------

	CVector3 PixelFromWorldPt(CVector3 worldPoint, unsigned int viewportWidth, unsigned int viewportHeight);

	CVector2 PixelSizeInWorldSpace(float Z, unsigned int viewportWidth, unsigned int viewportHeight);
	
//-------------------------------------
// Private members
//-------------------------------------
private:
	// Update the matrices used for the camera in the rendering pipeline
	void UpdateMatrices();

	// Postition and rotations for the camera (rarely scale cameras)
	CVector3 mPosition;
	CVector3 mRotation;

	// Camera settings: field of view, aspect ratio, near and far clip plane distances.
	// Note that the FOVx angle is measured in radians (radians = degrees * PI/180) from left to right of screen
	float mFOVx;
    float mAspectRatio;
	float mNearClip;
	float mFarClip;

	// Current view, projection and combined view-projection matrices (DirectX matrix type)
	CMatrix4x4 mWorldMatrix; // Easiest to treat the camera like a model and give it a "world" matrix...
	CMatrix4x4 mViewMatrix;  // ...then the view matrix used in the shaders is the inverse of its world matrix

	CMatrix4x4 mProjectionMatrix;     // Projection matrix holds the field of view and near/far clip distances
	CMatrix4x4 mViewProjectionMatrix; // Combine (multiply) the view and projection matrices together, which
	                                  // can sometimes save a matrix multiply in the shader (optional)
};

