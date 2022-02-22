#include "DX11Gui.h"
#include <sstream>

#include "..\Common/Camera.h"

#include <imgui.h>
#include <ImGuizmo.h>
#include <ImGuiFileBrowser.h>

#include "Objects/DX11GameObject.h"
#include "..\Window.h"

#include "DX11Scene.h"

#include "Objects/DX11SpotLight.h"
#include "Objects/DX11PointLight.h"
#include "Objects/DX11DirLight.h"
#include "Objects/DX11Light.h"
#include "Objects/DX11Plant.h"
#include "Objects/DX11Sky.h"
#include "GraphicsHelpers.h"
#include "../Utility/Input.h"

#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "../Common.h"

namespace DX11
{

	CDX11Gui::CDX11Gui(CDX11Engine* engine) :CGui(engine)
	{

		mEngine = engine;
		mScene = engine->GetScene();

		//initialize ImGui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows //super broken

		io.ConfigDockingWithShift = false;
		io.ConfigWindowsMoveFromTitleBarOnly = true;
		//io.Fonts->AddFontFromFileTTF("External\\imgui\\misc\\fonts\\Roboto-Light.ttf", 15);

		// Setup Platform/Renderer bindings
		ImGui_ImplDX11_Init(engine->GetDevice(), engine->GetContext());
		ImGui_ImplWin32_Init(engine->GetWindow()->GetHandle());
		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

	}

	void CDX11Gui::Begin(float& frameTime)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void CDX11Gui::Show(float& frameTime)
	{


	}




	CDX11Gui::~CDX11Gui()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void CDX11Gui::End() const {
		CGui::End();
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		ImGui::EndFrame();
		ImGui::UpdatePlatformWindows();
	}

	void CDX11Gui::DisplayShadowMaps() const
	{
		if (ImGui::Begin("ShadowMaps", 0, ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (ImGui::BeginTable("", 6))
			{
				for (const auto tx : mScene->mShadowsMaps)
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

	void CDX11Gui::AddObjectsMenu() const
	{
		
	}

	std::string ChooseTexture(bool& selected, imgui_addons::ImGuiFileBrowser fileDialog)
	{
		if (fileDialog.showFileDialog("Select Texture", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".jpg,.dds,.png"))
		{
			selected = false;
			return fileDialog.selected_fn;
		}
		return {};
	}

	void CDX11Gui::DisplayPropertiesWindow() const
	{
	}

	void CDX11Gui::DisplayObjects()
	{
		if (ImGui::Begin("Objects", 0, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar))
		{
			AddObjectsMenu();

			ImGui::Separator();

			//display for each model a button

			DisplayDeque(mScene->GetObjectManager()->mObjects);
			DisplayDeque(mScene->GetObjectManager()->mLights);
			DisplayDeque(mScene->GetObjectManager()->mSpotLights);
			DisplayDeque(mScene->GetObjectManager()->mDirLights);
			DisplayDeque(mScene->GetObjectManager()->mPointLights);
		}
		ImGui::End();

		if (mSelectedObj != nullptr)
		{
			DisplayPropertiesWindow();
		}
	}

	void CDX11Gui::DisplaySceneSettings(bool& b) const
	{
		if (ImGui::Begin("Scene Properties", &b))
		{
			ImGui::Checkbox("VSync", &mScene->GetLockFps());

			//ImGui::DragInt("PCF Samples", &mPcfSamples, 1, 0, 64);

			//static float bg[] = { mBackgroundColor.r,mBackgroundColor.g,mBackgroundColor.b,mBackgroundColor.a };

			static auto minMax = (&gPerFrameConstants.parallaxMinSamples, &gPerFrameConstants.parallaxMaxSamples);
			ImGui::DragFloat2("Min/Max parallax occlusion samples", minMax, 1.0f, 0.0f, D3D11_FLOAT32_MAX);

			//if (ImGui::ColorEdit4("Background Colour", bg))
			//{
			//	mBackgroundColor.Set(bg);
			//}

			ImGui::DragFloat("Depth Adjust", &gPerFrameConstants.gDepthAdjust, 0.0000001f, 0.0f, 0.1f, "%.7f");

		}
		ImGui::End();
	}


	template <class T>
	void CDX11Gui::DisplayDeque(std::deque<T*>& deque)
	{
		for (auto i = 0; i < deque.size(); ++i)
		{
			//if a button is pressed
			if (ImGui::Button(deque[i]->Name().c_str()))
			{
				//store the object pointer
				mSelectedObj = deque[i];
				return;
			}

			ImGui::SameLine();

			auto deleteLabel = "Delete##" + deque[i]->Name();

			//draw a button on the same line to delete the current object
			if (ImGui::Button(deleteLabel.c_str()))
			{
				//delete the current iterator from the container
				deque.erase(deque.begin() + i);
				mSelectedObj = nullptr;
				i--;
			}
		}
	}
}