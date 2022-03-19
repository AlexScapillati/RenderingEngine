#pragma once

#include "DX12DescriptorHeap.h"
#include "../Common/CGui.h"

namespace DX12
{
	class CDX12Engine;

	class CDX12Gui final : public CGui<CDX12Gui,CDX12Engine>
	{
	public:

		friend class CGui<CDX12Gui, CDX12Engine>;

		CDX12Gui(CDX12Engine* engine);

		// Disable copy/assignment/default constructors

		CDX12Gui() = delete;
		CDX12Gui(const CDX12Gui&) = delete;
		CDX12Gui(const CDX12Gui&&) = delete;
		CDX12Gui& operator=(const CDX12Gui&) = delete;
		CDX12Gui& operator=(const CDX12Gui&&) = delete;

		void BeginImpl() ;

		void EndImpl() ;

		~CDX12Gui() ;

	private:
		CDX12Engine* mEngine = nullptr;

		std::unique_ptr<CDX12DescriptorHeap> mDescHeap;

	};
}