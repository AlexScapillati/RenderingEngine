#pragma once

#include <deque>

#include "DX12DescriptorHeap.h"
#include "imgui.h"

namespace DX12
{
	class CDX12GameObject;
	class CDX12Engine;

	class CDX12Gui final
	{
	public:

		CDX12Gui(CDX12Engine* engine);

		// Disable copy/assignment/default constructors

		CDX12Gui() = delete;
		CDX12Gui(const CDX12Gui&) = delete;
		CDX12Gui(const CDX12Gui&&) = delete;
		CDX12Gui& operator=(const CDX12Gui&) = delete;
		CDX12Gui& operator=(const CDX12Gui&&) = delete;

		void Begin() ;

		void Show(float& frameTime);

		void AddObjectsMenu() const;


		void DisplayPropertiesWindow() const;

		void End() ;
		
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

		~CDX12Gui() ;

	private:
		CDX12Engine* mEngine = nullptr;

		std::unique_ptr<CDX12DescriptorHeap> mDescHeap;
		
		CDX12GameObject* mSelectedObj        = nullptr;
		bool         mViewportFullscreen = false;
		CVector2     mViewportWindowPos  = {};
	};
}
