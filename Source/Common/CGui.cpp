


#include "CGui.h"

#include <stdexcept>

#include "imgui.h"
#include "ImGuiFileBrowser.h"

#include "CGameObject.h"
#include "CLight.h"
#include "ImGuizmo.h"
#include "../Utility/Input.h"
#include "../Common.h"
#include "../Engine.h"
#include "../Window.h"
#include "../Common/CScene.h"
#include "../DX11/Objects/DX11GameObject.h"
#include "../Common/CGameObjectManager.h"
#include "../DX12/DX12Scene.h"
#include "../DX12/DX12AmbientMap.h"



CGui::CGui(IEngine* engine)
{
	

}

void CGui::Show(float& frameTime)
{
	
}

void CGui::AddObjectsMenu() const
{

}

void CGui::DisplayPropertiesWindow() const
{
	
}

void CGui::DisplayObjects()
{
}

void CGui::DisplaySceneSettings(bool& b) const
{
	
}

void CGui::DisplayShadowMaps() const
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
