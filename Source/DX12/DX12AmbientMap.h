#pragma once
#include "DX12DescriptorHeap.h"
#include "DX12Common.h" 

namespace DX12
{
	class CDX12ConstantBuffer;

	class CDX12AmbientMap
	{
		public:

			virtual ~CDX12AmbientMap() = default;

			CDX12AmbientMap() = delete;
			CDX12AmbientMap(CDX12Engine* e, int size, CDX12DescriptorHeap* srvHeap);

			void* RenderFromThis(CMatrix4x4* mat);

			void PrepareToRender();
			void PrepareToShow();


			bool mEnable;

			CDX12Engine*           mEngine;
			ComPtr<ID3D12Resource> mResource;

			std::unique_ptr<CDX12ConstantBuffer> mConstantBuffers[6];

			ComPtr<ID3D12Resource>               mDepthBufferResource;
			uint32_t                             mSrvHandle;
			uint32_t                             mRtvHandle[6];
			uint32_t                             mDsvHandle;
			std::unique_ptr<CDX12DescriptorHeap> mDsvHeap;
			std::unique_ptr<CDX12DescriptorHeap> mRtvHeap;
			CDX12DescriptorHeap*                 mSrvHeap;
			CD3DX12_VIEWPORT                     mVp;
			RECT                                 mScissorsRect;
			int                                  mSize;
	};
}
