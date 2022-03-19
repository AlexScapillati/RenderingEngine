#pragma once

#include <deque>
#include <string>

#include "imgui.h"
#include "../Math/CVector2.h"

class CScene;
class CGameObject;
class IEngine;

class CGui
{
	public:

		virtual ~CGui() = default;

		CGui(IEngine* engine);

		// Disable copy/assignment/default constructors
		CGui()                        = delete;
		CGui(const CGui&)             = delete;
		CGui(const CGui&&)            = delete;
		CGui& operator=(const CGui&)  = delete;
		CGui& operator=(const CGui&&) = delete;

		// ImGui Loop 

		virtual void Begin() = 0;
		virtual void Show(float& frameTime);
		virtual void End() = 0;

		// Helper functions

		void AddObjectsMenu() const;
		void DisplayPropertiesWindow() const;
		void DisplayObjects();
		void DisplaySceneSettings(bool& b) const;
		void DisplayShadowMaps() const;

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

	private:
		IEngine*     mEngine             = nullptr;
		CGameObject* mSelectedObj        = nullptr;
		bool         mViewportFullscreen = false;
		CVector2     mViewportWindowPos  = {};
};
