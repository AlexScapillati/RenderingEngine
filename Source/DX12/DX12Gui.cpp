
#include "DX12Gui.h"

#include "../Window.h"
#include "../Utility/Input.h"

#include "imgui.h"
#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_win32.h"

#include "ImGuiFileBrowser.h"

#include "DX12Scene.h"
#include "DX12DescriptorHeap.h"
#include "DX12GameObjectManager.h"
#include "CDX12Material.h"

#include "ImGuizmo.h"
#include "../Common/Camera.h"


CDX12Gui::CDX12Gui(CDX12Engine* engine)
{
	mEngine = engine;

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

	// Calculate offset on the srvdescriptorheap to store imgui font texture

	auto handle = mEngine->mSRVDescriptorHeap->Add();

	// Setup Platform/Renderer bindings
	if (!ImGui_ImplDX12_Init(engine->GetDevice(),
		engine->mNumFrames,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		engine->mSRVDescriptorHeap->mDescriptorHeap.Get(),
		handle.mCpu,
		handle.mGpu)
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
			mEngine->mMainScene.release();

			mEngine->mMainScene = std::make_unique<CDX12Scene>(mEngine, fileDialog.selected_fn);

			open = false;
		}

		if (save)
		{
			ImGui::OpenPopup("SaveScene");
		}

		if (fileDialog.showFileDialog("SaveScene", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(700, 310), ".xml"))
		{
			mEngine->mMainScene->Save(fileDialog.selected_fn);
			save = false;
		}

		ImGui::EndMainMenuBar();
	}


	ImGui::SetNextWindowPos({ 0,0 });
	ImGui::SetNextWindowSize({ (float)mEngine->GetWindow()->GetWindowWidth(),(float)mEngine->GetWindow()->GetWindowHeight() });

	if (ImGui::Begin("Engine", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		if (ImGui::Begin("Viewport", nullptr,
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

					mEngine->mMainScene->GetCamera()->ControlMouse(frameTime, delta, Key_W, Key_S, Key_A, Key_D);

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
			if ((size.x != mEngine->mMainScene->mViewportX || size.y != mEngine->mMainScene->mViewportY) && (size.x != 0 && size.y != 0))
			{
				//if they are different, resize the scene viewport
				mEngine->mMainScene->Resize(UINT(size.x), UINT(size.y));
			}

			//render the scene image to ImGui
			ImGui::Image(mEngine->mMainScene->GetTextureSrv(), size);

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
	static auto mCurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
	static bool showBounds = false;
	static bool bRename = false;
	static bool show = mSelectedObj != nullptr;

	if (ImGui::Begin("Properties", &show))
	{
		ImGui::Checkbox("Enabled", mSelectedObj->Enabled());

		ImGuizmo::Enable(mSelectedObj->Enabled());

		if (ImGui::Button("Rename"))
		{
			bRename = true;
		}

		if (bRename)
		{
			static char buffer[100] = { 0 };

			ImGui::InputText("Name", buffer, IM_ARRAYSIZE(buffer));

			if (ImGui::Button("OK"))
			{
				//set the name
				mSelectedObj->SetName(buffer);

				//hide the rename text input
				bRename = false;
			}
		}

		ImGui::Separator();

		//display the transform component
		ImGui::NewLine();
		ImGui::Separator();
		ImGui::Text("Transform");
		ImGui::Separator();

		if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
			mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
			mCurrentGizmoOperation = ImGuizmo::ROTATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
			mCurrentGizmoOperation = ImGuizmo::SCALE;

		static auto pos = mSelectedObj->Position();

		//get the direct access to the position of the model and display it
		if (ImGui::DragFloat3("Position", pos.GetValuesArray()))
		{
			mSelectedObj->SetPosition(pos);
		}

		//acquire the rotation array
		static auto rot = ToDegrees(mSelectedObj->Rotation());
		//display the rotation
		if (ImGui::DragFloat3("Rotation", rot.GetValuesArray()))
		{
			//set the rotation
			mSelectedObj->SetRotation(ToRadians(rot));
		}

		//display the scale array
		static auto scale = mSelectedObj->Scale();

		if (ImGui::DragFloat3("Scale", scale.GetValuesArray(), 0.1f, 0.01f, D3D12_FLOAT32_MAX, "%.3f"))
		{
			mSelectedObj->SetScale(scale);
		}

		auto& roughness = mEngine->mPerFrameConstants.roughness;
		auto& metalness = mEngine->mPerFrameConstants.metalness;
		ImGui::DragFloat("Roughness", &roughness, 0.001f);
		ImGui::DragFloat("Metalness", &metalness, 0.001f);

		//----------------------------------------------------------------
		// Object Specific settings
		//----------------------------------------------------------------

		if (const auto light = dynamic_cast<CDX12Light*>(mSelectedObj))
		{
			ImGui::Text("Specific settings");

			//light colour edit
			static bool colourPickerOpen = false;

			if (ImGui::Button("Edit Colour"))
			{
				colourPickerOpen = !colourPickerOpen;
			}

			if (colourPickerOpen)
			{
				if (ImGui::Begin("ColourPicker", &colourPickerOpen))
				{
					CVector3 col = light->Colour();
					if (ImGui::ColorPicker3("Colour", col.GetValuesArray()))
					{
						light->SetColour(col);
					}
				}
				ImGui::End();
			}

			//modify strength

			auto strength = light->Strength();

			if (ImGui::DragFloat("Strength", &strength, 0.1f, 0.0f, D3D12_FLOAT32_MAX))
			{
				light->SetStrength(strength);
			}
		}

		if (mSelectedObj->GetMeshes().size() > 1)
		{
			// Show Variations (if any)
			std::string previewValues;

			// Populate the string with the preview values separated with \0
			for (auto mesh : mSelectedObj->GetVariations())
			{
				previewValues += mesh + '\0';
			}

			// Show the combo selector
			static int selected = 0;
			if (ImGui::Combo("Mesh", &selected, previewValues.c_str()))
			{
				// If the user pressed load the new mesh
				mSelectedObj->LoadNewMesh(mSelectedObj->GetMeshes().at(selected));
			}

		}

		//display the textures
		ImGui::NewLine();
		ImGui::Text("Textures");

		const auto v = mSelectedObj->Material()->GetTextureSRV();

		for (const auto& id : v)
		{
			ImGui::Image(id, { 256,256 });
		}
	}
	ImGui::End();

	const auto pos = mViewportWindowPos;

	ImGui::GetWindowPos();

	ImGuizmo::SetRect(pos.x, pos.y, mEngine->mMainScene->GetViewportSize().x, mEngine->mMainScene->GetViewportSize().y);

	ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
	
	ImGuizmo::Manipulate(mEngine->mMainScene->GetCamera()->ViewMatrix().GetArray(), mEngine->mMainScene->GetCamera()->ProjectionMatrix().GetArray(),
		mCurrentGizmoOperation, ImGuizmo::WORLD, mSelectedObj->WorldMatrix().GetArray(), nullptr, nullptr, nullptr);

}

void CDX12Gui::DisplayObjects()
{
	if (ImGui::Begin("Objects", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar))
	{
		AddObjectsMenu();

		ImGui::Separator();

		//display for each model a button
		const auto objManager = mEngine->mMainScene->GetObjectManager();
		DisplayDeque(objManager->mObjects);
		DisplayDeque(objManager->mLights);
	}
	ImGui::End();

	if (mSelectedObj != nullptr)
	{
		DisplayPropertiesWindow();
	}
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

	mEngine->mSRVDescriptorHeap->Set();

	ImGui::Render();

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mEngine->mCommandList.Get());
	ImGui::EndFrame();
	ImGui::UpdatePlatformWindows();
}

template <class T>
void CDX12Gui::DisplayDeque(std::deque<T*>& deque)
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


CDX12Gui::~CDX12Gui()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
