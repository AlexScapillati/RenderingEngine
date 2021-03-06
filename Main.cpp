//--------------------------------------------------------------------------------------
// Entry point for the application
// Window creation code
//--------------------------------------------------------------------------------------


#pragma warning(disable : 4250)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <memory>
#include "Source/FactoryEngine.h"
#include "Source/Window.h"
#include "Source/Utility/Input.h"


std::unique_ptr<IEngine> myEngine;


//--------------------------------------------------------------------------------------
// The entry function for a Windows application is called wWinMain
//--------------------------------------------------------------------------------------
int APIENTRY wWinMain(
	_In_     HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_     LPWSTR    lpCmdLine,
	_In_     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance); // Stop warnings when not using some parameters
	UNREFERENCED_PARAMETER(lpCmdLine);

	try
	{
		if(FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE))) return 0;

		//create Engine
		myEngine = NewEngine(EDX12, hInstance, nCmdShow);

		myEngine->CreateScene(/*"Scene1.xml"*/);

		if (!myEngine->Update())
		{
			CoUninitialize();
			return 0;
		}
	}
	catch (const std::exception& e)
	{
		MessageBoxA(NULL,e.what(), NULL, MB_OK);
		return 0;
	}
	return 1;
}

// This is for ImGui to pick up input from the user
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//--------------------------------------------------------------------------------------
// Deal with a message from Windows. There are very many possible messages, such as keyboard/mouse input, resizing
// or minimizing windows, the system shutting down etc. We only deal with messages that we are interested in
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) return true;

	switch (message)
	{
	case WM_PAINT: // A necessary message to ensure the window content is displayed
	{
		PAINTSTRUCT ps;
		auto hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	}
	break;

	case WM_SIZE:
	{
		UINT X = LOWORD(lParam);
		UINT Y = HIWORD(lParam);

		if (myEngine)
		{
			myEngine->Resize(X, Y);
		}
		return 1;
	}

	case WM_DESTROY: // Another necessary message to deal with the window being closed
	{
		PostQuitMessage(0);
	}
	break;

	// The WM_KEYXXXX messages report keyboard input to our window.
	// This application has added some simple functions (not DirectX) to process these messages (all in Input.cpp/h)
	// so you don't need to change this code. Instead simply use KeyHit, KeyHeld etc.
	case WM_KEYDOWN:
	{
		KeyDownEvent(static_cast<KeyCode>(wParam));
		break;
	}

	case WM_KEYUP:
	{
		KeyUpEvent(static_cast<KeyCode>(wParam));
		break;
	}

	// The following WM_XXXX messages report mouse movement and button presses
	// Use KeyHit to get mouse buttons, GetMouseX, GetMouseY for its position
	case WM_MOUSEMOVE:
	{
		MouseMoveEvent(LOWORD(lParam), HIWORD(lParam));
		break;
	}
	case WM_LBUTTONDOWN:
	{
		KeyDownEvent(Mouse_LButton);
		break;
	}
	case WM_LBUTTONUP:
	{
		KeyUpEvent(Mouse_LButton);
		break;
	}
	case WM_RBUTTONDOWN:
	{
		KeyDownEvent(Mouse_RButton);
		break;
	}
	case WM_RBUTTONUP:
	{
		KeyUpEvent(Mouse_RButton);
		break;
	}
	case WM_MBUTTONDOWN:
	{
		KeyDownEvent(Mouse_MButton);
		break;
	}
	case WM_MBUTTONUP:
	{
		KeyUpEvent(Mouse_MButton);
		break;
	}

	// Any messages we don't handle are passed back to Windows default handling
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}