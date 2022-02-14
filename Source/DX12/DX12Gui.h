#pragma once

#include <deque>

#include "CDX12Common.h"

class CDX12Scene;
class CDX12Engine;
class CDX12GameObject;

class CDX12Gui
{
	public:

		CDX12Gui(CDX12Engine* engine);

		// Disable copy/assignment/default constructors

		CDX12Gui() = delete;
		CDX12Gui(const CDX12Gui&) = delete;
		CDX12Gui(const CDX12Gui&&) = delete;
		CDX12Gui& operator=(const CDX12Gui&) = delete;
		CDX12Gui& operator=(const CDX12Gui&&) = delete;

		void Begin(float& frameTime);

		void Show(float& frameTime);

		void AddObjectsMenu() const;

		void DisplayPropertiesWindow() const;

		void DisplayObjects();

		void DisplaySceneSettings(bool& b) const;

		void DisplayShadowMaps() const;

		bool IsSceneFullscreen() const;
		void End() const;

		template<class T>
		void DisplayDeque(std::deque<T*>& deque);

		~CDX12Gui();

	private:
		CDX12Engine*     mEngine             = nullptr;
		CDX12GameObject* mSelectedObj        = nullptr;
		bool             mViewportFullscreen = false;
		CVector2         mViewportWindowPos;
		
};
