#include "DX11Gui.h"
#include <sstream>

#include "..\Common/Camera.h"

#include <imgui.h>

#include "ImGuizmo.h"
#include "Objects/DX11GameObject.h"
#include "..\Window.h"


#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"

namespace DX11
{

	CDX11Gui::CDX11Gui(CDX11Engine* engine) :CGui(engine)
	{
		mEngine = engine;

		ImGui_ImplWin32_Init(engine->GetWindow()->GetHandle());
		ImGui_ImplDX11_Init(engine->GetDevice(), engine->GetContext());
	}

	void CDX11Gui::Begin()
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	CDX11Gui::~CDX11Gui()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void CDX11Gui::End() {
		
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		ImGui::UpdatePlatformWindows();
	}
}