#pragma once
#include "DX12DescriptorHeap.h"
#include "DX12Common.h" 

namespace DX12
{
	class CDX12AmbientMap
	{
		public:
			CDX12AmbientMap(CDX12Engine* e, int size, 
				CDX12DescriptorHeap* rtvHeap, CDX12DescriptorHeap* srvHeap, CDX12DescriptorHeap* dsvHeap);

			void* RenderFromThis(CMatrix4x4* mat);


			bool mEnable;

			CDX12Engine*           mEngine;
			ComPtr<ID3D12Resource> mResource;
			ComPtr<ID3D12Resource> mDepthBufferResource;
			SHandle                mSrvHandle;
			int                    mSrvIndex;
			SHandle                mRtvHandle[6];
			int                    mRtvIndex[6];
			SHandle                mDsvHandle;
			int                    mDsvIndex;
			CD3DX12_VIEWPORT       mVp;
			RECT                   mScissorsRect;
			int                    mSize;
	};
}
