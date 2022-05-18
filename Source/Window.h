#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class CWindow
{
public:

	CWindow(HINSTANCE hInstance, int nCmdShow);

	UINT GetWindowWidth();

	UINT GetWindowHeight();

	// only for internal use, this will not resize the window, it will only change internal values
	void SetWindowSize(UINT x, UINT y);

	HWND GetHandle();

	~CWindow();

private:

	UINT mWindowWidth = 1920;
	UINT mWindowHeight = 1080;

	HWND mHWnd;

};
