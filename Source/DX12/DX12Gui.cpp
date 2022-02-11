
#include "DX12Gui.h"

#include "../Window.h"

#include "imgui.h"
#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_win32.h"

#include "DX12Scene.h"
#include "ImGuiFileBrowser.h"

CDX12Gui::CDX12Gui(CDX12Engine* engine)
{
	mEngine = engine;
	mScene = engine->GetScene();

	//initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	//ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows //super broken

	//io.ConfigDockingWithShift = false;
	//io.ConfigWindowsMoveFromTitleBarOnly = true;
	//io.Fonts->AddFontFromFileTTF("External\\imgui\\misc\\fonts\\Roboto-Light.ttf", 15);

	// Calculate offset on the srvdescriptorheap to store imgui font texture
	
	const CD3DX12_CPU_DESCRIPTOR_HANDLE hCPUDescriptor(mEngine->mSRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), mEngine->mNumTextures , mEngine->mSRVSize);
	const CD3DX12_GPU_DESCRIPTOR_HANDLE hGPUDescriptor(mEngine->mSRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), mEngine->mNumTextures , mEngine->mSRVSize);

	mEngine->mNumTextures++;

	// Setup Platform/Renderer bindings
	if (!ImGui_ImplDX12_Init(engine->GetDevice(),
		engine->mNumFrames,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		engine->mSRVDescriptorHeap.Get(),
		hCPUDescriptor,
		hGPUDescriptor)
		|| 
		!ImGui_ImplWin32_Init(engine->GetWindow()->GetHandle()))
	{
		throw std::runtime_error("Impossible initialize ImGui");
	}

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
}

void CDX12Gui::Begin(float& frameTime)
{
	// Start the Dear ImGui frame
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void CDX12Gui::Show(float& frameTime)
{

	if (ImGui::BeginMainMenuBar())
	{
		static bool                           open = false;
		static bool                           save = false;
		static bool                           themeWindow = false;
		static bool                           sceneProperties = false;
		static imgui_addons::ImGuiFileBrowser fileDialog;

		if (ImGui::MenuItem("Open"))
		{
			open = true;
		}

		if (ImGui::MenuItem("Save"))
		{
			save = true;
		}

		if (ImGui::MenuItem("Theme"))
		{
			themeWindow = true;
		}

		if (ImGui::MenuItem("Scene Properties"))
		{
			sceneProperties = true;
		}

		if (sceneProperties)
		{
			DisplaySceneSettings(sceneProperties);
		}

		if (open)
		{
			ImGui::OpenPopup("OpenScene");
		}

		if (fileDialog.showFileDialog("OpenScene", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".xml"))
		{
			delete mScene;

			mScene = new CDX12Scene(mEngine, fileDialog.selected_fn);

			open = false;
		}

		if (save)
		{
			ImGui::OpenPopup("SaveScene");
		}

		if (fileDialog.showFileDialog("SaveScene", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(700, 310), ".xml"))
		{
			mScene->Save(fileDialog.selected_fn);
			save = false;
		}

		ImGui::EndMainMenuBar();
	}

	DisplayShadowMaps();

	mScene->DisplayPostProcessingEffects();


	auto vp = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos({ 0,0 });
	ImGui::SetNextWindowSize({ (float)mEngine->GetWindow()->GetWindowWidth(),(float)mEngine->GetWindow()->GetWindowHeight() });

	if (ImGui::Begin("Engine", 0, ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		if (ImGui::Begin("Viewport", 0,
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_MenuBar))
		{

			// Camera control 
			if (ImGui::IsWindowFocused())
			{
				if (KeyHeld(Mouse_RButton))
				{
					POINT mousePos;

					GetCursorPos(&mousePos);

					const CVector2 delta = { ImGui::GetMouseDragDelta(1).x, ImGui::GetMouseDragDelta(1).y };

					mScene->GetCamera()->ControlMouse(frameTime, delta, Key_W, Key_S, Key_A, Key_D);

					ImGui::ResetMouseDragDelta(1);
				}
			}

			if (ImGui::BeginMenuBar())
			{
				ImGui::MenuItem("Maximize", "", &mViewportFullscreen);
				ImGui::EndMenuBar();
			}

			//get the available region of the window
			auto size = ImGui::GetContentRegionAvail();

			if (mViewportFullscreen)
			{
				size = { (float)mEngine->GetWindow()->GetWindowWidth(), (float)mEngine->GetWindow()->GetWindowHeight() };
				ImGui::SetWindowSize(size);
			}

			//compare it with the scene viewport
			if ((size.x != mScene->mViewportX || size.y != mScene->mViewportY) && (size.x != 0 && size.y != 0))
			{
				//if they are different, resize the scene viewport
				mScene->Resize(UINT(size.x), UINT(size.y));
			}

			//render the scene image to ImGui
			//ImGui::Image(mScene->GetTextureSRV(), size);

			mViewportWindowPos.x = ImGui::GetWindowPos().x;
			mViewportWindowPos.y = ImGui::GetWindowPos().y;

		}
		ImGui::End();
	}
	ImGui::End();

	//render GUI
	if (!mViewportFullscreen)
		DisplayObjects();

}

void CDX12Gui::AddObjectsMenu() const
{
}

void CDX12Gui::DisplayPropertiesWindow() const
{
}

void CDX12Gui::DisplayObjects()
{
}

void CDX12Gui::DisplaySceneSettings(bool& b) const
{
}

void CDX12Gui::DisplayShadowMaps() const
{
}

bool CDX12Gui::IsSceneFullscreen() const
{
	return mViewportFullscreen;
}

void CDX12Gui::End() const
{
	ImGui::Render();

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mEngine->mCommandList.Get());
	ImGui::EndFrame();
	ImGui::UpdatePlatformWindows();
}

template <class T>
void CDX12Gui::DisplayDeque(std::deque<T*>& deque)
{
	if (ImGui::Begin("ShadowMaps", 0, ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		if (ImGui::BeginTable("", 6))
		{
			for (const auto tx : mScene->GetObjectManager()->mShadowsMaps)
			{
				const ImTextureID texId = tx;

				ImGui::TableNextColumn();

				ImGui::Image((void*)texId, { 256, 256 });
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();
}


CDX12Gui::~CDX12Gui()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
