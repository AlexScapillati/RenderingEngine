
#include "DX12Gui.h"

#include "../Window.h"

#include "imgui.h"
#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_win32.h"

#include "DX12DescriptorHeap.h"
#include "ImGuizmo.h"
#include "../Common/CScene.h"

#include "DX12Engine.h"

namespace DX12
{
	CDX12Gui::CDX12Gui(CDX12Engine* engine): CGui(engine)
	{
		mEngine = engine;

		// Calculate offset on the srvdescriptorheap to store imgui font texture
		auto handle = mEngine->mSRVDescriptorHeap->Get(mEngine->mSRVDescriptorHeap->Add());

		// Setup Platform/Renderer bindings
		if (!ImGui_ImplDX12_Init(engine->GetDevice(),
								CDX12Engine::mNumFrames,
								DXGI_FORMAT_R8G8B8A8_UNORM,
								mEngine->mSRVDescriptorHeap->mDescriptorHeap.Get(),
								handle.mCpu,
								handle.mGpu)
			||
			!ImGui_ImplWin32_Init(engine->GetWindow()->GetHandle())) { throw std::runtime_error("Impossible initialize ImGui"); }
		
	}

	void CDX12Gui::Begin()
	{
		// Start the Dear ImGui frame
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void CDX12Gui::End()
	{
		mEngine->mSRVDescriptorHeap->Set();
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mEngine->mCurrRecordingCommandList);
		ImGui::UpdatePlatformWindows();
	}
	

	CDX12Gui::~CDX12Gui()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
}
