//--------------------------------------------------------------------------------------
// Class encapsulating a camera
//--------------------------------------------------------------------------------------
// Holds position, rotation, near/far clip and field of view. These to a view and projection matrices as required

#include "Camera.h"

extern float ROTATION_SPEED;
extern float MOVEMENT_SPEED;

CCamera::CCamera(CVector3 position,
	CVector3 rotation,
	float fov,
	float aspectRatio,
	float nearClip,
	float farClip) :
	mPosition(position),
	mRotation(rotation),
	mFOVx(fov),
	mAspectRatio(aspectRatio),
	mNearClip(nearClip),
	mFarClip(farClip)
{
}

CVector3 CCamera::Position() const { return mPosition; }

CVector3 CCamera::Rotation() const { return mRotation; }

void CCamera::SetPosition(CVector3 position) { mPosition = position; }

void CCamera::SetRotation(CVector3 rotation) { mRotation = rotation; }

float CCamera::FOV() const { return mFOVx; }

float CCamera::NearClip() const { return mNearClip; }

float CCamera::FarClip() const { return mFarClip; }

void CCamera::SetFOV(float fov) { mFOVx = fov; }

void CCamera::SetNearClip(float nearClip) { mNearClip = nearClip; }

void CCamera::SetFarClip(float farClip) { mFarClip = farClip; }

CMatrix4x4& CCamera::WorldMatrix()
{
	UpdateMatrices();
	return mWorldMatrix;
}

CMatrix4x4& CCamera::ViewMatrix()
{
	UpdateMatrices();
	return mViewMatrix;
}

CMatrix4x4& CCamera::ProjectionMatrix()
{
	UpdateMatrices();
	return mProjectionMatrix;
}

CMatrix4x4& CCamera::ViewProjectionMatrix()
{
	UpdateMatrices();
	return mViewProjectionMatrix;
}

// Control the camera's position and rotation using keys provided
void CCamera::Control(float frameTime, KeyCode turnUp, KeyCode turnDown, KeyCode turnLeft, KeyCode turnRight,
	KeyCode moveForward, KeyCode moveBackward, KeyCode moveLeft, KeyCode moveRight)
{
	//**** ROTATION ****
	if (KeyHeld(Key_Down))
	{
		mRotation.x += ROTATION_SPEED * frameTime; // Use of frameTime to ensure same speed on different machines
	}
	if (KeyHeld(Key_Up))
	{
		mRotation.x -= ROTATION_SPEED * frameTime;
	}
	if (KeyHeld(Key_Right))
	{
		mRotation.y += ROTATION_SPEED * frameTime;
	}
	if (KeyHeld(Key_Left))
	{
		mRotation.y -= ROTATION_SPEED * frameTime;
	}

	//**** LOCAL MOVEMENT ****
	if (KeyHeld(Key_D))
	{
		mPosition.x += MOVEMENT_SPEED * frameTime * mWorldMatrix.e00; // See comments on local movement in UpdateCube code above
		mPosition.y += MOVEMENT_SPEED * frameTime * mWorldMatrix.e01;
		mPosition.z += MOVEMENT_SPEED * frameTime * mWorldMatrix.e02;
	}
	if (KeyHeld(Key_A))
	{
		mPosition.x -= MOVEMENT_SPEED * frameTime * mWorldMatrix.e00;
		mPosition.y -= MOVEMENT_SPEED * frameTime * mWorldMatrix.e01;
		mPosition.z -= MOVEMENT_SPEED * frameTime * mWorldMatrix.e02;
	}
	if (KeyHeld(Key_W))
	{
		mPosition.x += MOVEMENT_SPEED * frameTime * mWorldMatrix.e20;
		mPosition.y += MOVEMENT_SPEED * frameTime * mWorldMatrix.e21;
		mPosition.z += MOVEMENT_SPEED * frameTime * mWorldMatrix.e22;
	}
	if (KeyHeld(Key_S))
	{
		mPosition.x -= MOVEMENT_SPEED * frameTime * mWorldMatrix.e20;
		mPosition.y -= MOVEMENT_SPEED * frameTime * mWorldMatrix.e21;
		mPosition.z -= MOVEMENT_SPEED * frameTime * mWorldMatrix.e22;
	}
}

// Control the camera's position and rotation using keys provided
void CCamera::ControlMouse(float frameTime, CVector2 delta, KeyCode moveForward, KeyCode moveBackward, KeyCode moveLeft, KeyCode moveRight)
{
	//**** ROTATION ****

	mRotation.x += (delta.y * ROTATION_SPEED * frameTime);
	mRotation.y += (delta.x * ROTATION_SPEED * frameTime);

	//**** LOCAL MOVEMENT ****
	if (KeyHeld(moveRight))
	{
		mPosition.x += MOVEMENT_SPEED * frameTime * mWorldMatrix.e00;
		mPosition.y += MOVEMENT_SPEED * frameTime * mWorldMatrix.e01;
		mPosition.z += MOVEMENT_SPEED * frameTime * mWorldMatrix.e02;
	}
	if (KeyHeld(moveLeft))
	{
		mPosition.x -= MOVEMENT_SPEED * frameTime * mWorldMatrix.e00;
		mPosition.y -= MOVEMENT_SPEED * frameTime * mWorldMatrix.e01;
		mPosition.z -= MOVEMENT_SPEED * frameTime * mWorldMatrix.e02;
	}
	if (KeyHeld(moveForward))
	{
		mPosition.x += MOVEMENT_SPEED * frameTime * mWorldMatrix.e20;
		mPosition.y += MOVEMENT_SPEED * frameTime * mWorldMatrix.e21;
		mPosition.z += MOVEMENT_SPEED * frameTime * mWorldMatrix.e22;
	}
	if (KeyHeld(moveBackward))
	{
		mPosition.x -= MOVEMENT_SPEED * frameTime * mWorldMatrix.e20;
		mPosition.y -= MOVEMENT_SPEED * frameTime * mWorldMatrix.e21;
		mPosition.z -= MOVEMENT_SPEED * frameTime * mWorldMatrix.e22;
	}

}


void CCamera::SetAspectRatio(float r)
{
	mAspectRatio = r;
}


// Update the matrices used for the camera in the rendering pipeline
void CCamera::UpdateMatrices()
{
	// "World" matrix for the camera - treat it like a model at first
	mWorldMatrix = MatrixRotationZ(mRotation.z) * MatrixRotationX(mRotation.x) * MatrixRotationY(mRotation.y) * MatrixTranslation(mPosition);

	// View matrix is the usual matrix used for the camera in shaders, it is the inverse of the world matrix (see lectures)
	mViewMatrix = InverseAffine(mWorldMatrix);

	// Projection matrix, how to flatten the 3D world onto the screen (needs field of view, near and far clip, aspect ratio)
	const auto tanFOVx = std::tan(mFOVx * 0.5f);
	const auto scaleX = 1.0f / tanFOVx;
	const auto scaleY = mAspectRatio / tanFOVx;
	const auto scaleZa = mFarClip / (mFarClip - mNearClip);
	const auto scaleZb = -mNearClip * scaleZa;

	mProjectionMatrix = { scaleX,   0.0f,    0.0f,   0.0f,
							0.0f, scaleY,    0.0f,   0.0f,
							0.0f,   0.0f, scaleZa,   1.0f,
							0.0f,   0.0f, scaleZb,   0.0f };

	// The view-projection matrix combines the two matrices usually used for the camera into one, which can save a multiply in the shaders (optional)
	mViewProjectionMatrix = mViewMatrix * mProjectionMatrix;
}



//-----------------------------------------------------------------------------
// Camera picking
//-----------------------------------------------------------------------------

// Return pixel coordinates corresponding to given world point when viewing from this
// camera. Pass the viewport width and height. The returned CVector3 contains the pixel
// coordinates in x and y and the Z-distance to the world point in z. If the Z-distance
// is less than the camera near clip (use NearClip() member function), then the world
// point is behind the camera and the 2D x and y coordinates are to be ignored.
CVector3 CCamera::PixelFromWorldPt(CVector3 worldPoint, unsigned int viewportWidth, unsigned int viewportHeight)
{
	CVector3 pixelPoint;

	UpdateMatrices();

	// Transform world point into camera space and return immediately if point is behind camera near clip (it won't be on screen - no 2D pixel position)
	CVector4 cameraPt = CVector4(worldPoint, 1.0f) * mViewMatrix;
	if (cameraPt.z < mNearClip)
	{
		return { 0, 0, cameraPt.z };
	}

	// Now transform into viewport (2D) space
	CVector4 viewportPt = cameraPt * mProjectionMatrix;

	viewportPt.x /= viewportPt.w;
	viewportPt.y /= viewportPt.w;

	float x = (viewportPt.x + 1.0f) * viewportWidth * 0.5f;
	float y = (1.0f - viewportPt.y) * viewportHeight * 0.5f;

	return { x, y, cameraPt.z };
}


// Return the size of a pixel in world space at the given Z distance. Allows us to convert the 2D size of areas on the screen to actualy sizes in the world
// Pass the viewport width and height
CVector2 CCamera::PixelSizeInWorldSpace(float Z, unsigned int viewportWidth, unsigned int viewportHeight)
{
	CVector2 size;

	// Size of the entire viewport in world space at the near clip distance - uses same geometry work that was shown in the camera picking lecture
	CVector2 viewportSizeAtNearClip;
	viewportSizeAtNearClip.x = 2 * mNearClip * std::tan(mFOVx * 0.5f);
	viewportSizeAtNearClip.y = viewportSizeAtNearClip.x / mAspectRatio;

	// Size of the entire viewport in world space at the given Z distance
	CVector2 viewportSizeAtZ = viewportSizeAtNearClip * Z / mNearClip;

	// Return world size of single pixel at given Z distance
	return { viewportSizeAtZ.x / viewportWidth, viewportSizeAtZ.y / viewportHeight };
}

