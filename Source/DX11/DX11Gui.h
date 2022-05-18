#pragma once

#include "../Common/CGui.h"

namespace DX11
{
	class CDX11Engine;

	class CDX11Gui : public CGui
	{
		public:
	
			CDX11Gui(CDX11Engine* engine);

			// Disable copy/assignment/default constructors

			CDX11Gui() = delete;
			CDX11Gui(const CDX11Gui&) = delete;
			CDX11Gui(const CDX11Gui&&) = delete;
			CDX11Gui& operator=(const CDX11Gui&) = delete;
			CDX11Gui& operator=(const CDX11Gui&&) = delete;

			void Begin() override;

			void End() override;

			~CDX11Gui() override;

		private:
			CDX11Engine*     mEngine             = nullptr;
	};
	
}
