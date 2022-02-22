#pragma once

#include <deque>

#include "DX11Engine.h"
#include "../Common/CGameObject.h"

#include "../Common/CGui.h"

namespace DX11
{
	class CDX11Gui : public CGui
	{
		public:
	
			CDX11Gui(CDX11Engine* engine);

			void Begin(float& frameTime) override;

			void Show(float& frameTime) override;

			void AddObjectsMenu() const override;

			void DisplayPropertiesWindow() const override;

			void DisplayObjects() override;

			void DisplaySceneSettings(bool& b) const override;

			void DisplayShadowMaps() const override;

			template<class T>
			void DisplayDeque(std::deque<T*>& deque);

			~CDX11Gui() override;

			void End() const override;

		private:
			CDX11Engine*     mEngine             = nullptr;
			CDX11Scene*      mScene              = nullptr;
			CGameObject* mSelectedObj        = nullptr;
			bool             mViewportFullscreen = false;
			CVector2         mViewportWindowPos  = {};
	};
	
}
