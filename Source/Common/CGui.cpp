


#include "CGui.h"

#include <stdexcept>

#include "imgui.h"
#include "ImGuiFileBrowser.h"
#include <windef.h>

#include "CGameObject.h"
#include "ImGuizmo.h"
#include "../Utility/Input.h"
#include "../Common.h"
#include "../Engine.h"
#include "../Window.h"
#include "../Common/CScene.h"
#include "../DX11/Objects/DX11GameObject.h"

void CGui::Begin(float& frameTime)
{
}

void CGui::Show(float& frameTime)
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

			mScene = mEngine->CreateScene(fileDialog.selected_fn);

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
					ImVec2 mousePos = ImGui::GetCursorPos();

					CVector2 delta = { ImGui::GetMouseDragDelta(1).x, ImGui::GetMouseDragDelta(1).y };

					if (KeyHeld(Key_LShift))
						MOVEMENT_SPEED = 100.0f;
					else
						MOVEMENT_SPEED = 50.0f;
					/*
								WIP When the mouse is on the borders of the window set its position the the opposite border

								RECT winRect;

								GetWindowRect(gHWnd, &winRect);

								if (mousePos.x > winRect.right) SetCursorPos(winRect.left, mousePos.y);

								else if (mousePos.x < winRect.left) SetCursorPos(winRect.right, mousePos.y);

								else if (mousePos.y > winRect.bottom) SetCursorPos(mousePos.x, winRect.top);

								else if (mousePos.y < winRect.top) SetCursorPos(mousePos.x, winRect.bottom);

								else
					*/

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
			if ((size.x != mScene->GetViewportX() || size.y != mScene->GetViewportY()) && (size.x != 0 && size.y != 0))
			{
				//if they are different, resize the scene viewport
				mScene->Resize(UINT(size.x), UINT(size.y));
			}

			//render the scene image to ImGui
			ImGui::Image(mScene->GetTextureSRV(), size);

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

void CGui::AddObjectsMenu() const
{
	static imgui_addons::ImGuiFileBrowser fileDialog;
	static bool                           addObj = false;

	enum EAddType
	{
		None,
		Simple,
		Pbr,
		Plant,
		Sky,
		SimpleLight,
		SpotLight,
		DirLight,
		OmniLight
	};

	static EAddType addType = None;

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Add"))
		{
			if (ImGui::MenuItem("Simple Object"))
			{
				addType = Simple;
			}
			if (ImGui::MenuItem("Pbr Object"))
			{
				addType = Pbr;
			}
			if (ImGui::MenuItem("Plant"))
			{
				addType = Plant;
			}
			if (ImGui::MenuItem("Sky"))
			{
				addType = Sky;
			}
			if (ImGui::BeginMenu("Lights"))
			{
				if (ImGui::MenuItem("Simple Light"))
				{
					addType = SimpleLight;
				}
				if (ImGui::MenuItem("Spot Light"))
				{
					addType = SpotLight;
				}
				if (ImGui::MenuItem("Directional Light"))
				{
					addType = DirLight;
				}
				if (ImGui::MenuItem("Omnidirectional Light"))
				{
					addType = OmniLight;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	static bool        selectMesh = false;
	static std::string mesh;
	static std::string tex;
	static std::string name;
	static bool        selectTexture = false;
	static bool        selectNormal = false;
	static bool        selectFolder = false;

	if (addType != None)
	{
		//open a window
		if (ImGui::Begin("Add Object"), &addObj, ImGuiWindowFlags_NoDocking)
		{
			//if the model is not pbr
			if (addType != Pbr && addType != Plant)
			{
				//show a button that will open a file dialog to open a mesh
				if (ImGui::Button("Add Mesh"))
				{
					selectMesh = true;
				}

				//show the mesh file name
				if (!mesh.empty())
				{
					ImGui::SameLine();
					ImGui::Text(mesh.c_str());
				}

				//show a button that will open a file dialog to open a texture
				if (ImGui::Button("Add Texture"))
				{
					selectTexture = true;
				}
			}
			else
			{
				//if the model is pbr just show a button for opening a file dialog to open a mesh file that contains the id of the model
				if (ImGui::Button("Add ID"))
				{
					selectMesh = true;
				}

				// Show a butto to select a folder

				if (ImGui::Button("Select folder"))
				{
					selectFolder = true;
				}

				//show the mesh file name
				if (!mesh.empty())
				{
					ImGui::SameLine();
					ImGui::Text(mesh.c_str());
				}
			}

			//show the texture filename
			if (!tex.empty())
			{
				ImGui::SameLine();
				ImGui::Text(tex.c_str());
			}

			// colour button for the light objects
			static CVector3 col;
			static float    strenght;

			//if the model is a light (hence not a simple object or pbr)
			if (addType == SimpleLight || addType == DirLight || addType == OmniLight || addType == SpotLight)
			{
				ImGui::ColorEdit3("LightColor", col.GetValuesArray());
				ImGui::DragFloat("Strength", &strenght);
			}

			//show button that will add the object in the object manager
			if (ImGui::Button("Add"))
			{
				addObj = false;

				//get the actual name without the extension
				const auto pos = mesh.find('.');

				name = mesh.substr(0, pos);

				//depending on the type create the corresponding object type

				switch (addType)
				{
				case Simple:

					if (!mesh.empty() && !tex.empty())
					{
						const auto newObj = mEngine->CreateObject(mesh, name, tex);
						mScene->GetObjectManager()->AddObject(newObj);

						addObj = false;
					}

					break;

				case Pbr:

					if (!mesh.empty())
					{
						const auto newObj = mEngine->CreateObject(mesh, name);
						mScene->GetObjectManager()->AddObject(newObj);

						addObj = false;
					}

					break;
				case Sky:

					if (!mesh.empty() && !tex.empty())
					{
						const auto newObj = mEngine->CreateSky(mesh, name, tex);
						mScene->GetObjectManager()->AddSky(newObj);

						addObj = false;
					}
					break;
				case Plant:

					if (!mesh.empty())
					{
						const auto newObj = mEngine->CreatePlant(mesh, name);

						mScene->GetObjectManager()->AddPlant(newObj);

						addObj = false;
					}

					break;

				case SimpleLight:

					if (!mesh.empty() & !tex.empty())
					{
						const auto newObj = mEngine->CreateLight(mesh, name, tex, col, strenght);
						mScene->GetObjectManager()->AddLight(newObj);

						addObj = false;
					}

					break;

				case SpotLight:

					if (!mesh.empty() & !tex.empty())
					{
						const auto newObj = mEngine->CreateSpotLight(mesh, name, tex, col, strenght);
						mScene->GetObjectManager()->AddSpotLight(newObj);

						addObj = false;
					}

					break;

				case DirLight:

					if (!mesh.empty() & !tex.empty())
					{
						const auto newObj = mEngine->CreateDirectionalLight(mesh, name, tex, col, strenght);
						mScene->GetObjectManager()->AddDirLight(newObj);

						addObj = false;
					}

					break;

				case OmniLight:

					if (!mesh.empty() & !tex.empty())
					{
						const auto newObj = mEngine->CreatePointLight(mesh, name, tex, col, strenght);
						mScene->GetObjectManager()->AddPointLight(newObj);

						addObj = false;
					}

					break;
				}
			}
		}
		ImGui::End();
	}

	//open file dialog code
	if (selectMesh)
	{
		ImGui::OpenPopup("Select Mesh");
	}
	if (selectTexture)
	{
		ImGui::OpenPopup("Select Texture");
	}
	if (selectFolder)
	{
		ImGui::OpenPopup("Select Folder");
	}

	if (fileDialog.showFileDialog("Select Mesh", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".x,.fbx"))
	{
		selectMesh = false;
		mesh = fileDialog.selected_fn;
	}

	if (fileDialog.showFileDialog("Select Texture", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".jpg,.dds,.png"))
	{
		selectTexture = false;
		tex = fileDialog.selected_fn;
	}

	if (fileDialog.showFileDialog("Select Folder", imgui_addons::ImGuiFileBrowser::DialogMode::SELECT, ImVec2(700, 310)))
	{
		selectFolder = false;
		mesh = fileDialog.selected_fn;
	}
}

void CGui::DisplayPropertiesWindow() const
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

		if (ImGui::Button("Duplicate Selected Obj"))
		{
			//WIP
			try
			{
				if (auto light = dynamic_cast<CLight*>(mSelectedObj))
				{
					if (auto spotLight = dynamic_cast<CSpotLight*>(mSelectedObj))
					{
						//TODO
						auto o = mEngine->CreateSpotLight(mSelectedObj->MeshFileNames(), mSelectedObj->Name(), "", light->GetColour(), light->GetStrength());
						mScene->GetObjectManager()->AddSpotLight(o);
					}
					else if (auto dirLight = dynamic_cast<CDirectionalLight*>(mSelectedObj))
					{
						auto obj = mEngine->CreateDirectionalLight(mSelectedObj->MeshFileNames(), mSelectedObj->Name(), "", light->GetColour(), light->GetStrength());
						mScene->GetObjectManager()->AddDirLight(obj);
					}
					else if (auto omniLight = dynamic_cast<CPointLight*>(mSelectedObj))
					{
						auto obj = mEngine->CreatePointLight(mSelectedObj->MeshFileNames(), mSelectedObj->Name(), "", light->GetColour(), light->GetStrength());
						mScene->GetObjectManager()->AddPointLight(obj);
					}
					else
					{
						auto obj = mEngine->CreateLight(mSelectedObj->MeshFileNames(), mSelectedObj->Name(), "", light->GetColour(), light->GetStrength());
						mScene->GetObjectManager()->AddLight(obj);
					}
				}
				else
				{
					if (auto plant = dynamic_cast<CPlant*>(mSelectedObj))
					{
						auto obj = mEngine->CreatePlant(mSelectedObj->MeshFileNames(), mSelectedObj->Name());
						mScene->GetObjectManager()->AddPlant(obj);
					}
					else if (auto sky = dynamic_cast<CSky*>(mSelectedObj))
					{
						auto obj = mEngine->CreateSky(mSelectedObj->MeshFileNames(), mSelectedObj->Name(), "");
						mScene->GetObjectManager()->AddSky(obj);
					}
					else
					{
						auto obj = mEngine->CreateObject(mSelectedObj->MeshFileNames(), mSelectedObj->Name(), "");
						mScene->GetObjectManager()->AddObject(obj);
					}
				}
			}
			catch (std::exception& e)
			{
				throw std::runtime_error(e.what());
			}
		}


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

		if (ImGui::DragFloat3("Scale", scale.GetValuesArray(), 0.1f, 0.0001f))
		{
			mSelectedObj->SetScale(scale);
		}

		ImGui::DragFloat("ParallaxDepth", &mSelectedObj->ParallaxDepth(), 0.0001f, 0.0f, 1.0f, "%.6f");

		ImGui::DragFloat("Roughness", &mSelectedObj->Roughness(), 0.001f);
		ImGui::DragFloat("Metalness", &mSelectedObj->Metalness(), 0.001f);


		//----------------------------------------------------------------
		// Object Specific settings
		//----------------------------------------------------------------

		if (const auto light = dynamic_cast<CLight*>(mSelectedObj))
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
					ImGui::ColorPicker3("Colour", light->GetColour().GetValuesArray());
				}
				ImGui::End();
			}

			//modify strength
			ImGui::DragFloat("Strength", &light->GetStrength(), 0.1f);

			//if it is a spotlight let it modify few things
			if (const auto spotLight = dynamic_cast<CSpotLight*>(mSelectedObj))
			{

				//modify cone angle
				ImGui::DragFloat("Cone Angle", &spotLight->GetConeAngle(), 1.0f, 0.0f, 180.0f);

				//modify shadow map size
				static int size = (int)std::log2(spotLight->GetShadowMapSize());
				if (ImGui::DragInt("ShadowMapsSize", &size, 1, 1, 14))
				{
					spotLight->SetShadowMapsSize((int)pow<int, int>(2, size));
				}
			}
			else if (const auto dirLight = dynamic_cast<CDirectionalLight*>(mSelectedObj))
			{
				//modify shadow map size
				static int size = (int)std::log2(dirLight->GetShadowMapSize());
				if (ImGui::DragInt("ShadowMapsSize", &size, 1, 1, 14))
				{
					dirLight->SetShadowMapSize((int)pow(2, size));
				}

				//modify near clip and far clip
				static auto nearClip = dirLight->GetNearClip();
				static auto farClip = dirLight->GetFarClip();

				if (ImGui::DragFloat("NearClip", &nearClip, 0.01f, 0.0f, 10.0f))
				{
					dirLight->SetNearClip(nearClip);
				}

				if (ImGui::DragFloat("FarClip", &farClip, 10.0f))
				{
					dirLight->SetFarClip(farClip);
				}

				//modify the size of the matrix

				static auto width = dirLight->GetWidth();
				static auto height = dirLight->GetHeight();

				if (ImGui::DragFloat("Height", &height, 10.0f, 1.0f))
				{
					dirLight->SetHeight(height);
				}

				if (ImGui::DragFloat("Width", &width, 10.0f, 1.0f))
				{
					dirLight->SetWidth(width);
				}
			}
			else if (const auto point = dynamic_cast<CPointLight*>(mSelectedObj))
			{
				//modify shadow map size
				static int size = (int)std::log2(point->GetShadowMapSize());
				if (ImGui::DragInt("ShadowMapsSize", &size, 1, 1, 12))
				{
					point->SetShadowMapSize((int)pow<int, int>(2, size));
				}
			}
		}

		if (mSelectedObj->GetMeshes().size() > 1)
		{
			// Show Variations (if any)
			std::string previewValues = "";

			// Populate the string with the preview values separated with \0
			for (const auto& mesh : mSelectedObj->GetVariations())
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


		// Ambient Map Settings
		ImGui::Separator();
		ImGui::Text("Ambient Map");
		ImGui::Separator();

		ImGui::Checkbox("Toggle ambient Map", &dynamic_cast<DX11::CDX11GameObject*>(mSelectedObj)->AmbientMapEnabled());

		//display the ambient map (if any)
		if (dynamic_cast<DX11::CDX11GameObject*>(mSelectedObj)->AmbientMapEnabled())
		{
			ImGui::NewLine();
			ImGui::Text("AmbientMap");

			static int size = (int)std::log2(dynamic_cast<DX11::CDX11GameObject*>(mSelectedObj)->AmbientMap()->Size());
			if (ImGui::DragInt("Size (base 2)", &size, 1, 1, 12))
			{
				dynamic_cast<DX11::CDX11GameObject*>(mSelectedObj)->AmbientMap()->SetSize((UINT)pow(2, size));
			}
		}
	}
	ImGui::End();

	const auto pos = mViewportWindowPos;

	ImGui::GetWindowPos();

	ImGuizmo::SetRect(pos.x, pos.y, mScene->GetViewportSize().x, mScene->GetViewportSize().y);

	ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

	static float bounds[] =
	{
		0.0f,0.0f,0.0f,
		1.f, 1.f, 1.f
	};

	ImGuizmo::Manipulate(mScene->GetCamera()->ViewMatrix().GetArray(), mScene->GetCamera()->ProjectionMatrix().GetArray(),
		mCurrentGizmoOperation, ImGuizmo::WORLD, mSelectedObj->WorldMatrix().GetArray(), 0, 0, showBounds ? bounds : 0);

}
