#pragma once

#include <deque>
#include <string>

#include "imgui.h"
#include "ImGuiFileBrowser.h"
#include "ImGuizmo.h"
#include "../Math/CVector2.h"

#include "../Engine.h"

class CGameObject;

template <typename Impl, typename EngineImpl>
class CGui
{
	public:

		~CGui() = default;

		// Disable copy/assignment/default constructors
		CGui(const CGui&)             = delete;
		CGui(const CGui&&)            = delete;
		CGui& operator=(const CGui&)  = delete;
		CGui& operator=(const CGui&&) = delete;

		// ImGui Loop 

		void Begin() { impl()->BeginImpl; }
		void End() { impl()->EndImpl; }


	//-----------------------------------------------------------
	//
	// Common Usage
	//
	//-----------------------------------------------------------

		CGui(IEngine<EngineImpl>* engine) : mEngine(engine)
		{
			//initialize ImGui
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();

			ImGuiIO& io = ImGui::GetIO();
			(void)io;
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
			//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows //super broken

			io.ConfigDockingWithShift            = false;
			io.ConfigWindowsMoveFromTitleBarOnly = true;
			//io.Fonts->AddFontFromFileTTF("External\\imgui\\misc\\fonts\\Roboto-Light.ttf", 15);

			// Setup Platform/Renderer bindings
			// Setup Dear ImGui style
			ImGui::StyleColorsDark();
		}

		void Show(float& frameTime)

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
					delete mEngine->GetScene();

					mEngine->CreateScene(fileDialog.selected_fn);

					open = false;
				}

				if (save)
				{
					ImGui::OpenPopup("SaveScene");
				}

				if (fileDialog.showFileDialog("SaveScene", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(700, 310), ".xml"))
				{
					mEngine->GetScene()->Save(fileDialog.selected_fn);
					save = false;
				}

				ImGui::EndMainMenuBar();
			}

			DisplayShadowMaps();

			mEngine->GetScene()->DisplayPostProcessingEffects();

			ImGui::SetNextWindowPos({ 0,0 });
			ImGui::SetNextWindowSize({ (float)mEngine->GetWindow()->GetWindowWidth(),(float)mEngine->GetWindow()->GetWindowHeight() - 10 });

			if (ImGui::Begin("Engine", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus))
			{
				if (ImGui::Begin("Viewport", nullptr,
					ImGuiWindowFlags_NoScrollbar |
					ImGuiWindowFlags_NoCollapse |
					ImGuiWindowFlags_MenuBar))
				{

					ImGuizmo::SetDrawlist();

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

							mEngine->GetScene()->GetCamera()->ControlMouse(frameTime, delta, Key_W, Key_S, Key_A, Key_D);

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
					if ((size.x != mEngine->GetScene()->GetViewportX() || size.y != mEngine->GetScene()->GetViewportY()) && (size.x != 0 && size.y != 0))
					{
						//if they are different, resize the scene viewport
						mEngine->GetScene()->Resize(UINT(size.x), UINT(size.y));
					}

					//render the scene image to ImGui
					ImGui::Image(mEngine->GetScene()->GetTextureSRV(), size);

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


		// Helper functions

		void AddObjectsMenu() const

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
								mEngine->GetObjManager()->AddObject(newObj);

								addObj = false;
							}

							break;

						case Pbr:

							if (!mesh.empty())
							{
								const auto newObj = mEngine->CreateObject(mesh, name);
								mEngine->GetObjManager()->AddObject(newObj);

								addObj = false;
							}

							break;
						case Sky:

							if (!mesh.empty() && !tex.empty())
							{
								const auto newObj = mEngine->CreateSky(mesh, name, tex);
								mEngine->GetObjManager()->AddSky(newObj);

								addObj = false;
							}
							break;
						case Plant:

							if (!mesh.empty())
							{
								const auto newObj = mEngine->CreatePlant(mesh, name);

								mEngine->GetObjManager()->AddPlant(newObj);

								addObj = false;
							}

							break;

						case SimpleLight:

							if (!mesh.empty() & !tex.empty())
							{
								const auto newObj = mEngine->CreateLight(mesh, name, tex, col, strenght);

								addObj = false;
							}

							break;

						case SpotLight:

							if (!mesh.empty() & !tex.empty())
							{
								const auto newObj = mEngine->CreateSpotLight(mesh, name, tex, col, strenght);

								addObj = false;
							}

							break;

						case DirLight:

							if (!mesh.empty() & !tex.empty())
							{
								const auto newObj = mEngine->CreateDirectionalLight(mesh, name, tex, col, strenght);

								addObj = false;
							}

							break;

						case OmniLight:

							if (!mesh.empty() & !tex.empty())
							{
								const auto newObj = mEngine->CreatePointLight(mesh, name, tex, col, strenght);
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


		void DisplayPropertiesWindow() const

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
							if (dynamic_cast<CSpotLight*>(mSelectedObj))
							{
								//TODO
								auto o = mEngine->CreateSpotLight(mSelectedObj->MeshFileNames(), mSelectedObj->Name(), "", light->GetColour(), light->GetStrength());
							}
							else if (dynamic_cast<CDirectionalLight*>(mSelectedObj))
							{
								auto obj = mEngine->CreateDirectionalLight(mSelectedObj->MeshFileNames(), mSelectedObj->Name(), "", light->GetColour(), light->GetStrength());
							}
							else if (dynamic_cast<CPointLight*>(mSelectedObj))
							{
								auto obj = mEngine->CreatePointLight(mSelectedObj->MeshFileNames(), mSelectedObj->Name(), "", light->GetColour(), light->GetStrength());
							}
							else
							{
								auto obj = mEngine->CreateLight(mSelectedObj->MeshFileNames(), mSelectedObj->Name(), "", light->GetColour(), light->GetStrength());
							}
						}
						else
						{
							if (auto plant = dynamic_cast<CPlant*>(mSelectedObj))
							{
								auto obj = mEngine->CreatePlant(mSelectedObj->MeshFileNames(), mSelectedObj->Name());
							}
							else if (auto sky = dynamic_cast<CSky*>(mSelectedObj))
							{
								auto obj = mEngine->CreateSky(mSelectedObj->MeshFileNames(), mSelectedObj->Name(), "");
							}
							else
							{
								auto obj = mEngine->CreateObject(mSelectedObj->MeshFileNames(), mSelectedObj->Name(), "");
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

						ImGui::Text("ShadowMap:");
						ImGui::Image(spotLight->GetSRV(), { 128,128 });

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


				//display the ambient map (if any)
				auto obj = dynamic_cast<DX11::CDX11GameObject*>(mSelectedObj);
				if (obj)
				{
					ImGui::Checkbox("Toggle ambient Map", &dynamic_cast<DX11::CDX11GameObject*>(mSelectedObj)->AmbientMapEnabled());
					if (obj->AmbientMapEnabled())
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
				else
				{
					ImGui::Text("Not supported in DX12");
				}
			}
			ImGui::End();

			const auto pos = mViewportWindowPos;

			ImGuizmo::Enable(true);
			ImGuizmo::SetRect(pos.x, pos.y, mEngine->GetScene()->GetViewportSize().x, mEngine->GetScene()->GetViewportSize().y);
			ImGuizmo::Manipulate(mEngine->GetScene()->GetCamera()->ViewMatrix().GetArray(), mEngine->GetScene()->GetCamera()->ProjectionMatrix().GetArray(),
				mCurrentGizmoOperation, ImGuizmo::WORLD, mSelectedObj->WorldMatrix().GetArray());

		}


		void DisplayObjects()

		{
			if (ImGui::Begin("Objects", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar))
			{
				AddObjectsMenu();
				ImGui::Separator();
				//display for each model a button
				const auto objManager = mEngine->GetObjManager();
				DisplayDeque(objManager->mObjects);
				DisplayDeque(objManager->mLights);
				DisplayDeque(objManager->mSpotLights);
				DisplayDeque(objManager->mDirLights);
				DisplayDeque(objManager->mPointLights);
			}
			ImGui::End();
			if (mSelectedObj != nullptr)
			{
				DisplayPropertiesWindow();
			}
		}

		void DisplaySceneSettings(bool& b) const
		{
			if (ImGui::Begin("Scene Properties", &b))
			{
				ImGui::Checkbox("VSync", &mEngine->GetScene()->GetLockFps());
			}
			ImGui::End();
		}


		void DisplayShadowMaps() const
		{
			if (ImGui::Begin("ShadowMaps", 0, ImGuiWindowFlags_NoBringToFrontOnFocus))
			{
				/*if (ImGui::BeginTable("", 6))
				{
					for (const auto tx : mScene->GetObjectManager())
					{
						const ImTextureID texId = tx;
						ImGui::TableNextColumn();
						ImGui::Image((void*)texId, { 256, 256 });
					}
					ImGui::EndTable();
				}*/
			}
			ImGui::End();
		}

		bool IsSceneFullscreen() const { return mViewportFullscreen; }


		template <class T>
		void DisplayDeque(std::deque<T*>& deque)
			{
				for (auto i = 0; i < deque.size(); ++i)
				{
					//if a button is pressed
					if (ImGui::Button(std::string(deque[i]->Name() + "##" + std::to_string(i)).c_str()))
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



		std::string ChooseTexture(bool& selected, imgui_addons::ImGuiFileBrowser fileDialog)
		{
			if (fileDialog.showFileDialog("Select Texture", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".jpg,.dds,.png"))
			{
				selected = false;
				return fileDialog.selected_fn;
			}
			return {};
		}

	private:

		IEngine<EngineImpl>* mEngine;

		CGameObject* mSelectedObj        = nullptr;
		bool         mViewportFullscreen = false;
		CVector2     mViewportWindowPos  = {};

		Impl* impl() { return static_cast<Impl*>(this); }
	
};
