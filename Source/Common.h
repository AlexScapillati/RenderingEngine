#pragma once


	//--------------------------------------------------------------------------------------
	// Global Variables
	//--------------------------------------------------------------------------------------
	// Make global Variables from various files available to other files. "extern" means
	// this variable is defined in another file somewhere. We should use classes and avoid
	// use of globals, but done this way to keep code simpler so the DirectX content is
	// clearer. However, try to architect your own code in a better way.

	// Input constants
inline float ROTATION_SPEED = 1.f;
inline float MOVEMENT_SPEED = 50.0f;