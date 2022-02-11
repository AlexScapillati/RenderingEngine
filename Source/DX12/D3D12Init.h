
#pragma once

#include "DescriptorHeap.h"
#include "DX12Engine.h"

#include "DX12Scene.h"

#include "../Window.h"


inline void CDX12Engine::CheckRayTracingSupport() const
{
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};

	const auto hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5));

	if (hr != S_OK) throw std::runtime_error("Error");

	if (options5.RaytracingTier < D3D12_RAYTRACING_TIER_1_0) throw std::runtime_error("RayTracing Not Supported");

}

inline void CDX12Engine::EnableDebugLayer() const
{
#if defined(_DEBUG)

	// Always enable the debug layer before doing anything DX12 related
	// so all possible errors generated while creating DX12 objects
	// are caught by the debug layer.
	ComPtr<ID3D12Debug1> debugInterface;

	if (D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)) != S_OK)
	{
		throw std::runtime_error("Impossible to enable debug layer");
	}

	debugInterface->EnableDebugLayer();

#endif
}

static void ReportLiveObjects()
{
	IDXGIDebug1* dxgiDebug;
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));
	dxgiDebug->EnableLeakTrackingForThread();
	//dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	//dxgiDebug->Release();
}

inline ComPtr<IDXGIAdapter4> GetAdapter(bool useWarp)
{
	ComPtr<IDXGIFactory4> dxgiFactory;

	UINT createFactoryFlags = 0;

#if defined(_DEBUG)

	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

#endif

	if (FAILED(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory))))
	{
		throw std::runtime_error("Could not create debug adapter");
	}

	ComPtr<IDXGIAdapter1> dxgiAdapter1;
	ComPtr<IDXGIAdapter4> dxgiAdapter4;

	if (useWarp)
	{
		if (FAILED(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1))))
		{
			throw std::runtime_error("Could not get warp adapter");
		}

		if (FAILED(dxgiAdapter1.As(&dxgiAdapter4))) { throw std::runtime_error("Could not parse adapter"); }
	}
	else
	{
		SIZE_T maxDedicatedVideoMemory = 0;

		for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
			dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

			// Check to see if the adapter can create a D3D12 device without actually 
			// creating it. The adapter with the largest dedicated video memory
			// is favored.

			if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
				SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr
				)) &&
				dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
			{
				maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
				ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
			}
		}
	}
	return dxgiAdapter4;
}

inline ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> adapter)
{
	ComPtr<ID3D12Device2> d3d12Device2;
	if (FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2))))
	{
		throw std::runtime_error("Error Creating device");
	}

	// Enable debug messages in debug mode.

#if defined(_DEBUG)

	ComPtr<ID3D12InfoQueue> pInfoQueue;

	if (SUCCEEDED(d3d12Device2.As(&pInfoQueue)))
	{
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);

		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);

		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		// Suppress whole categories of messages

		//D3D12_MESSAGE_CATEGORY Categories[] = {};

		// Suppress messages based on their severity level

		D3D12_MESSAGE_SEVERITY Severities[] =

		{

			D3D12_MESSAGE_SEVERITY_INFO

		};

		// Suppress individual messages by their ID

		D3D12_MESSAGE_ID DenyIds[] = {

			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
			// I'm really not sure how to avoid this message.

			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
			// This warning occurs when using capture frame while graphics debugging.

			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
			// This warning occurs when using capture frame while graphics debugging.

		};

		D3D12_INFO_QUEUE_FILTER NewFilter = {};

		//NewFilter.DenyList.NumCategories = _countof(Categories);
		//NewFilter.DenyList.pCategoryList = Categories;

		NewFilter.DenyList.NumSeverities = _countof(Severities);
		NewFilter.DenyList.pSeverityList = Severities;
		NewFilter.DenyList.NumIDs = _countof(DenyIds);
		NewFilter.DenyList.pIDList = DenyIds;

		if (FAILED(pInfoQueue->PushStorageFilter(&NewFilter)))
		{
			throw std::runtime_error("Error");
		}
	}

#endif

	return d3d12Device2;
}


inline bool CheckTearingSupport()
{
	BOOL allowTearing = FALSE;

	// Rather than create the DXGI 1.5 factory interface directly, we create the
	// DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
	// graphics debugging tools which will not support the 1.5 factory interface 
	// until a future update.
	ComPtr<IDXGIFactory4> factory4;
	if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
	{
		ComPtr<IDXGIFactory5> factory5;
		if (SUCCEEDED(factory4.As(&factory5)))
		{
			if (FAILED(factory5->CheckFeatureSupport(
				DXGI_FEATURE_PRESENT_ALLOW_TEARING,
				&allowTearing, sizeof(allowTearing))))
			{
				allowTearing = FALSE;
			}
		}
	}

	return allowTearing == TRUE;
}



inline uint64_t CDX12Engine::Signal()
{
	const uint64_t fenceValueForSignal = ++mFenceValue;
	if (FAILED(mCommandQueue->Signal(mFence.Get(), fenceValueForSignal)))
	{
		throw std::runtime_error("Error");
	}

	return fenceValueForSignal;
}



inline HANDLE CreateEventHandle()
{
	const HANDLE fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent && "Failed to create fence event.");

	return fenceEvent;
}


inline void CDX12Engine::InitD3D()
{
	// Create viewport
	mViewport = CD3DX12_VIEWPORT(
		0.0f,
		0.0f,
		static_cast<FLOAT>(mWindow->GetWindowWidth()),
		static_cast<FLOAT>(mWindow->GetWindowHeight()));

	mScissorRect = CD3DX12_RECT(0, 0, mWindow->GetWindowWidth(), mWindow->GetWindowHeight());

	// Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
	// Using this awareness context allows the client area of the window 
	// to achieve 100% scaling while still allowing non-client window content to 
	// be rendered in a DPI sensitive fashion.
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	EnableDebugLayer();

	const ComPtr<IDXGIAdapter4> dxgiAdapter4 = GetAdapter(mUseWarp);

	mDevice = CreateDevice(dxgiAdapter4);

	NAME_D3D12_OBJECT(mDevice);

	// Create command queue
	{
		D3D12_COMMAND_QUEUE_DESC desc;
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		if (FAILED(mDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&mCommandQueue))))
		{
			throw std::runtime_error("Error creating command queue");
		}

		NAME_D3D12_OBJECT(mCommandQueue);
	}

	// Create swap chain
	{
		ComPtr<IDXGISwapChain4> dxgiSwapChain4;
		ComPtr<IDXGIFactory4>   dxgiFactory4;
		UINT                    createFactoryFlags;

#if defined(_DEBUG)
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#else
		createFactoryFlags = 0;
#endif

		if (FAILED(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4))))
		{
			throw std::runtime_error("Error create DXGIFactory");
		}

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		
		swapChainDesc.Width = mWindow->GetWindowWidth();
		swapChainDesc.Height = mWindow->GetWindowHeight();
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.SampleDesc = { 1,0 };
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = mNumFrames;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		// It is recommended to always allow tearing if tearing support is available.
		swapChainDesc.Flags = CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		ComPtr<IDXGISwapChain1> swapChain1;
		if (FAILED(
			dxgiFactory4->CreateSwapChainForHwnd(
				mCommandQueue.Get(),
				mWindow->GetHandle(),
				&swapChainDesc,
				nullptr,
				nullptr,
				&swapChain1))) {
			throw std::runtime_error("Error creating swap chain");
		}

		// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
		// will be handled manually.
		if (FAILED(dxgiFactory4->MakeWindowAssociation(mWindow->GetHandle(), DXGI_MWA_NO_ALT_ENTER)))
		{
			throw std::runtime_error("Error");
		}

		if (FAILED(swapChain1.As(&mSwapChain))) { throw std::runtime_error("Error casting swap chain"); }
	}

	mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = mNumFrames;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		if (FAILED(mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRTVDescriptorHeap))))
		{
			throw std::runtime_error("Error rtv heap");
		}

		NAME_D3D12_OBJECT(mRTVDescriptorHeap);

		mRTVSize = mDevice->GetDescriptorHandleIncrementSize(mRTVDescriptorHeap->GetDesc().Type);

	}

	// Create depth stencil view
	{
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = mNumFrames;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		if (FAILED(mDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&mDSVDescriptorHeap))))
		{
			throw std::runtime_error("Error dsv heap");
		}

		NAME_D3D12_OBJECT(mDSVDescriptorHeap);


		mSDSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	}

	// Describe and create a shader resource view (SRV) descriptor heap.
	{
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		if (FAILED(mDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSRVDescriptorHeap))))
		{
			throw std::runtime_error("Error srv heap"); 
		}

		NAME_D3D12_OBJECT(mSRVDescriptorHeap);

		mSRVSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	{
		// Describe and create a constant buffer view (CBV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
		cbvHeapDesc.NumDescriptors = mNumFrames;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		if (FAILED(mDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mCBVDescriptorHeap))))
		{
			throw std::runtime_error("Error cbv heap");
		}

		NAME_D3D12_OBJECT(mCBVDescriptorHeap);


		mCBVSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	{
		// Describe and create a sampler descriptor heap
		D3D12_DESCRIPTOR_HEAP_DESC samplerDesc = {};
		samplerDesc.NumDescriptors = 1;
		samplerDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		samplerDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		ThrowIfFailed(mDevice->CreateDescriptorHeap(&samplerDesc, IID_PPV_ARGS(&mSamplerDescriptorHeap)));

		NAME_D3D12_OBJECT(mSamplerDescriptorHeap);

		mSDSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}


	// Create command allocators
	{
		for (int i = 0; i < mNumFrames; ++i)
		{
			if (FAILED(
				mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocators[i]))))
			{
				throw std::runtime_error("Error creating command allocator");
			}

			NAME_D3D12_OBJECT_INDEXED(mCommandAllocators, i);
		}
	}

	// Create command list 
	{
		ThrowIfFailed(
			mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocators[mCurrentBackBufferIndex].
				Get(), nullptr, IID_PPV_ARGS(&mCommandList)));

		mCommandList->Close();

		NAME_D3D12_OBJECT(mCommandList);
	}

	// Create fence
	{
		if (FAILED(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence))))
		{
			throw std::runtime_error("Error creating fence");
		}

		NAME_D3D12_OBJECT(mFence);

		mFenceEvent = CreateEventHandle();
	}

	// Create the constant buffer.
	{
		constexpr UINT constantBufferSize = sizeof(DX12Common::PerFrameConstants);
		
		const auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

		const auto buffer = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);

		ThrowIfFailed(
			mDevice->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&buffer,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&mPerFrameConstantBuffer)));

		// Describe and create a constant buffer view.
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = mPerFrameConstantBuffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = constantBufferSize;
		mDevice->CreateConstantBufferView(&cbvDesc, mCBVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());


		// Map and initialize the constant buffer. We don't unmap this until the
		// app closes. Keeping things mapped for the lifetime of the resource is okay.
		const CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
		ThrowIfFailed(mPerFrameConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mFrameCbvDataBegin)));

		CopyBuffers();
	}

	atexit(&ReportLiveObjects);
}


inline void CDX12Engine::InitFrameDependentResources()
{
	// Create frame resources
	{
		for (int i = 0; i < mNumFrames; ++i)
		{
			if (FAILED(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mBackBuffers[i]))))
			{
				throw std::runtime_error("Error getting the current swap chain buffer");
			}

			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
			rtvHandle.Offset(i, mRTVSize);

			mDevice->CreateRenderTargetView(mBackBuffers[i].Get(), nullptr, rtvHandle);

			mRTVTop++;
			
			NAME_D3D12_OBJECT_INDEXED(mBackBuffers, i);
		}
	}
}