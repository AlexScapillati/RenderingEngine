#pragma once

#include "DX12Common.h"

namespace DX12
{
	class CDX12Engine;

	class CDX12RootSignature
	{

	public:

		virtual ~CDX12RootSignature() = default;
<<<<<<< HEAD
		void     Set(ID3D12RootSignature* rootSignature);
=======
>>>>>>> parent of a9c1de14 (revert commit)

		CDX12RootSignature(CDX12Engine* engine, CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc);

		ID3D12RootSignature* Get();


	private:

		CDX12Engine* mEngine;
		ComPtr<ID3D12RootSignature> mRootSignature;

	};

	class CDX12PBRRootSignature
	{
	public:

		virtual ~CDX12PBRRootSignature() = default;

		CDX12PBRRootSignature(CDX12Engine* engine);

		CDX12Engine* mEngine;
		std::unique_ptr<CDX12RootSignature> mRootSignature;
	};


	class CDX12SkyRootSignature
	{
	public:

		virtual ~CDX12SkyRootSignature() = default;

		CDX12SkyRootSignature(CDX12Engine* engine);

		CDX12Engine* mEngine;
		std::unique_ptr<CDX12RootSignature> mRootSignature;
	};

	class CDX12DepthOnlyRootSignature
	{
	public:

		virtual ~CDX12DepthOnlyRootSignature() = default;

		CDX12DepthOnlyRootSignature(CDX12Engine* engine);

		CDX12Engine* mEngine;
		std::unique_ptr<CDX12RootSignature> mRootSignature;
	};
}
