#pragma once

#include <deque>

#include "../Common/CGui.h"

namespace DX12
{

class CDX12Scene;
class CDX12Engine;
class CDX12GameObject;

	class CDX12Gui final : public CGui
	{
	public:

		CDX12Gui(CDX12Engine* engine);

		// Disable copy/assignment/default constructors

		CDX12Gui() = delete;
		CDX12Gui(const CDX12Gui&) = delete;
		CDX12Gui(const CDX12Gui&&) = delete;
		CDX12Gui& operator=(const CDX12Gui&) = delete;
		CDX12Gui& operator=(const CDX12Gui&&) = delete;

		void Begin(float& frameTime) override;

		void Show(float& frameTime) override;

		void AddObjectsMenu() const override;

		void DisplayPropertiesWindow() const override;

		void DisplayObjects() override;

		void DisplaySceneSettings(bool& b) const override;

		void DisplayShadowMaps() const override;

		void End() const;

		template<class T>
		void DisplayDeque(std::deque<T*>& deque);

		~CDX12Gui();

	private:
		CDX12Engine* mEngine = nullptr;
		CDX12GameObject* mSelectedObj = nullptr;
		bool             mViewportFullscreen = false;
		CVector2         mViewportWindowPos;

	};
}