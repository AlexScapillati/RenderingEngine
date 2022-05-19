#pragma once

#include <deque>

#include "../Common/CGui.h"
#include "DX12DescriptorHeap.h"
#include "imgui.h"

namespace DX12
{
	class CDX12GameObject;
	class CDX12Engine;

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

		void Begin() override;

		void End() override;

		~CDX12Gui() override;

	private:

		CDX12Engine* mEngine = nullptr;
	};
}
