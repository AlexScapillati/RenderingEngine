#pragma once

#include "../Common/CGui.h"

namespace DX11
{
	class CDX11Engine;

	class CDX11Gui : public CGui<CDX11Gui,CDX11Engine>
	{
		public:

			friend class CGui<CDX11Gui, CDX11Engine>;
	
			CDX11Gui(CDX11Engine* engine);

			// Disable copy/assignment/default constructors

			CDX11Gui() = delete;
			CDX11Gui(const CDX11Gui&) = delete;
			CDX11Gui(const CDX11Gui&&) = delete;
			CDX11Gui& operator=(const CDX11Gui&) = delete;
			CDX11Gui& operator=(const CDX11Gui&&) = delete;

			void BeginImpl();

			void EndImpl();

			~CDX11Gui();

		private:
			CDX11Engine*     mEngine             = nullptr;
	};
	
}
