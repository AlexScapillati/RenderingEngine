#pragma once

#include "..\Engine.h"

<<<<<<< HEAD
#include "DX12ConstantBuffer.h"
=======
#include "DX12Common.h"
#include "imgui.h"
>>>>>>> parent of 7bb1619 (Merge branch 'main' into TryingPolymorphism)

// https://www.3dgep.com/learning-directx-12-1/#GPU_Synchronization


namespace DX12
{
<<<<<<< HEAD
	class CDX12DepthOnlyPSO;
	class CDX12SkyPSO;
	class CDX12PBRPSO;
=======

>>>>>>> parent of 7bb1619 (Merge branch 'main' into TryingPolymorphism)
	class CDX12RenderTarget;
	class CDX12DescriptorHeap;
	class CDX12ConstantBuffer;
	class CDX12Scene;
	class CDX12Gui;
	class CDX12Shader;

	class CDX12Engine final : public IEngine
	{
	public:

		CDX12Engine() = delete;
		CDX12Engine(const CDX12Engine&) = delete;
		CDX12Engine(const CDX12Engine&&) = delete;
		CDX12Engine& operator=(const CDX12Engine&) = delete;
		CDX12Engine& operator=(const CDX12Engine&&) = delete;


		CDX12Engine(HINSTANCE hInstance, int nCmdShow);

		~CDX12Engine() override;

		// Inherited via IEngine
<<<<<<< HEAD
<<<<<<< HEAD
		bool UpdateImpl() ;
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< Updated upstream
<<<<<<< HEAD
<<<<<<< HEAD

		void ResizeImpl(UINT x, UINT y) ;

		void FinalizeFrameImpl();
=======
>>>>>>> parent of 32b3477 (Merge pull request #4 from AlexScapillati/TryingPolymorphism)
=======
		bool Update() override;
>>>>>>> parent of 78525fa (Merge pull request #2 from AlexScapillati/TryingPolymorphism)

		// Inherited via IEngine
		void Resize(UINT x, UINT y) override;
		void CreatePipelineStateObjects();

<<<<<<< HEAD
=======

		// Inherited via IEngine
		void ResizeImpl(UINT x, UINT y) ;

		void FinalizeFrameImpl();

		void CreatePipelineStateObjects();

<<<<<<< HEAD
=======
=======
=======
>>>>>>> parent of cbe79af (IDK)

		// Inherited via IEngine
		void ResizeImpl(UINT x, UINT y) ;

		void FinalizeFrameImpl();

		void CreatePipelineStateObjects();

<<<<<<< HEAD
>>>>>>> parent of 32b3477 (Merge pull request #4 from AlexScapillati/TryingPolymorphism)
=======
=======

		// Inherited via IEngine
=======

>>>>>>> parent of 100d753 (Merge pull request #3 from AlexScapillati/TryingPolymorphism)
=======

<<<<<<< HEAD
>>>>>>> parent of b0bd427 (Up)
=======
		// Inherited via IEngine
<<<<<<< Updated upstream
		void Resize(UINT x, UINT y) override;
=======
>>>>>>> parent of 7bb1619 (Merge branch 'main' into TryingPolymorphism)
		void ResizeImpl(UINT x, UINT y) ;

		void FinalizeFrameImpl();

		void CreatePipelineStateObjects();
>>>>>>> Stashed changes


<<<<<<< HEAD
<<<<<<< HEAD
>>>>>>> parent of cbe79af (IDK)

>>>>>>> Stashed changes
=======
>>>>>>> parent of 100d753 (Merge pull request #3 from AlexScapillati/TryingPolymorphism)
=======
>>>>>>> parent of b0bd427 (Up)
=======
		void FinalizeFrame() override;
>>>>>>> parent of 78525fa (Merge pull request #2 from AlexScapillati/TryingPolymorphism)
=======
		bool Update() override;

		// Inherited via IEngine
		void Resize(UINT x, UINT y) override;

		void FinalizeFrame() override;
>>>>>>> parent of e03ed59 (Static Polymorphic attemp)
		void Present();


		//--------------------------
		// Setters / Getters
		//--------------------------

		ID3D12Device2* GetDevice() const;
		ImTextureID    GetSceneTex() const;


		//--------------------------
		// DirectX 12 Variables
		//--------------------------

		static const uint32_t mNumFrames = 3;

		ComPtr<ID3D12Device2> mDevice;

		ComPtr<IDXGISwapChain4> mSwapChain;

		std::unique_ptr<CDX12RenderTarget> mBackBuffers[mNumFrames];
		ComPtr<ID3D12Resource> mDepthStencils[mNumFrames];

		CD3DX12_VIEWPORT mViewport;
		CD3DX12_RECT mScissorRect;

		/*
		 * A Command List is used to issue copy, compute (dispatch), or draw commands.
		 * In DirectX 12 commands issued to the command list are not executed immediately -
		 * like they are with the DirectX 11 immediate context.
		 * All command lists in DirectX 12 are deferred; that is, the commands in a command list -
		 * are only run on the GPU after they have been executed on a command queue.
		 */
		ComPtr<ID3D12GraphicsCommandList2> mCommandList;

		ComPtr<ID3D12CommandQueue> mCommandQueue;

		/*
		 * The ID3D12CommandAllocator serves as the backing memory for recording the GPU commands into a command list.
		 * Unlike the command list, a command allocator cannot be reused
		 * unless all of the commands that have been recorded into the command allocator have finished executing on the GPU.
		 * Attempting to reset a command allocator before the command queue has finished executing
		 * those commands will result in a COMMAND_ALLOCATOR_SYNC error by the debug layer.
		 * The mCommandAllocators array variable is used to store the reference to the command allocators.
		 * There must be at least one command allocator per render frame that is “in-flight”
		 * (at least one per back buffer of the swap chain).
		 */
		ComPtr<ID3D12CommandAllocator> mCommandAllocators[mNumFrames];

		/*
		 * In previous versions of DirectX, RTVs were created one at a time.
		 * Since DirectX 12, RTVs are now stored in descriptor heaps.
		 * A descriptor heap can be visualized as an array of descriptors(views).
		 * A view simply describes a resource that resides in GPU memory.
		 * The mRTVDescriptorHeap variable is used to store the descriptor heap that
		 * contains the render target views for the swap chain back buffers.
		*/
		std::unique_ptr<CDX12DescriptorHeap> mRTVDescriptorHeap;
		std::unique_ptr<CDX12DescriptorHeap> mSRVDescriptorHeap;
		std::unique_ptr<CDX12DescriptorHeap> mDSVDescriptorHeap;
		std::unique_ptr<CDX12DescriptorHeap> mCBVDescriptorHeap;
		std::unique_ptr<CDX12DescriptorHeap> mSamplerDescriptorHeap;


		/*
		 * The index holding the current back buffer that will need to be presented is stored here
		 */

		UINT mCurrentBackBufferIndex;

		/*
		 * The software rasterizer allows the graphics programmer
		 * to access the full set of advanced rendering features that
		 * may not be available in the hardware (for example, when running on older GPUs).
		 * The WARP device can also be used to verify the results of a rendering technique
		 * if the quality of the vendor supplied display driver is in question.
		*/
		bool mUseWarp = false;


		//-------------------------
		// Synchronization objects
		//-------------------------

		/*
		 * The Fence object is used to synchronize commands issued to the Command Queue.
		 * The fence stores a single value that indicates the last value that was used to signal the fence.
		 * Although it is possible to use the same fence object with multiple command queues,
		 * it is not reliable to ensure the proper synchronization of commands across command queues.
		 * Therefore, it is advised to create at least one fence object for each command queue.
		 * Multiple command queues can wait on a fence to reach a specific value,
		 * but the fence should only be allowed to be signaled from a single command queue.
		 * In addition to the fence object, the application must also track a fence value that is used to signal the fence.
		 */
		ComPtr<ID3D12Fence> mFence;

		// The next fence value to signal the command queue next is stored in the mFenceValue variable.
		uint64_t mFenceValue = 0;

		/*
		 * For each rendered frame that could be “in-flight” on the command queue,
		 * the fence value that was used to signal the command queue needs to be tracked to guarantee
		 * that any resources that are still being referenced by the command queue are not overwritten.
		 * The g_FrameFenceValues array variable is used to keep track of the fence values
		 * that were used to signal the command queue for a particular frame.
		 */

		uint64_t mFrameFenceValues[mNumFrames] = {};

		// The mFenceEvent variable is a handle to an OS event object that will be used to receive the notification that the fence has reached a specific value.
		HANDLE mFenceEvent;

		//----------------------------------------
		// Constant Buffers
		//-----------------------------------------

		PerFrameConstants mPerFrameConstants;
		PerFrameLights mPerFrameLights;

		std::unique_ptr<CDX12ConstantBuffer> mPerFrameConstantBuffer;
		std::unique_ptr<CDX12ConstantBuffer> mPerFrameLightsConstantBuffer;

		void CopyBuffers();

		//----------------------------------------
		// Shaders
		//-----------------------------------------

		SShader mPbrPixelShader;
		SShader mPbrVertexShader;
		SShader mPbrNormalPixelShader;
		SShader mPbrNormalVertexShader;
		SShader mDepthOnlyPixelShader;
		SShader mDepthOnlyNormalPixelShader;
		SShader mBasicTransformVertexShader;
		SShader mTintedTexturePixelShader;
		SShader mSkyPixelShader;
		SShader mSkyVertexShader;

		std::unique_ptr<CDX12Shader> vs;
		std::unique_ptr<CDX12Shader> ps;

		//----------------------------------------
		// DirectX Functions
		//-----------------------------------------

		void InitD3D();

		void InitFrameDependentResources();

		void LoadDefaultShaders();

		void CheckRayTracingSupport() const;

		void EnableDebugLayer() const;

		void Flush();

		uint64_t Signal();

		void MidFrame();

		void InitializeFrame();


		// Execute a command list.
		// Returns the fence value to wait for for this command list.
		uint64_t ExecuteCommandList(ID3D12GraphicsCommandList2* commandList);

		void WaitForFenceValue(ComPtr<ID3D12Fence>       fence,
			uint64_t                  fenceValue,
			HANDLE                    fenceEvent,
			std::chrono::milliseconds duration = std::chrono::milliseconds::max());

		void			   CreateScene(std::string fileName) override;
		CGameObject*       CreateObject(const std::string& mesh, const std::string& name, const std::string& diffuseMap, CVector3 position, CVector3 rotation, float scale) override;
		CSky*              CreateSky(const std::string& mesh, const std::string& name, const std::string& diffuseMap, CVector3 position, CVector3 rotation, float scale) override;
		CPlant*            CreatePlant(const std::string& id, const std::string& name, CVector3 position, CVector3 rotation, float scale) override;
		CGameObject*       CreateObject(const std::string& dirPath, const std::string& name, CVector3 position, CVector3 rotation, float scale) override;
		CLight*            CreateLight(const std::string& mesh, const std::string& name, const std::string& diffuseMap, const CVector3& colour, const float& strength, CVector3 position, CVector3 rotation, float scale) override;
		CSpotLight*        CreateSpotLight(const std::string& mesh, const std::string& name, const std::string& diffuseMap, const CVector3& colour, const float& strength, CVector3 position, CVector3 rotation, float scale) override;
		CDirectionalLight* CreateDirectionalLight(const std::string& mesh, const std::string& name, const std::string& diffuseMap, const CVector3& colour, const float& strength, CVector3 position, CVector3 rotation, float scale) override;
		CPointLight*       CreatePointLight(const std::string& mesh, const std::string& name, const std::string& diffuseMap, const CVector3& colour, const float& strength, CVector3 position, CVector3 rotation, float scale) override;
	};
}