#pragma once

#include "DX12GameObject.h"

#include "../../Common/CLight.h"


namespace DX12
{
	class CDX12Light : public CDX12GameObject, public CLight
	{
		public:
			CDX12Light(CDX12Engine*       engine,
					   const std::string& mesh,
					   const std::string& name,
					   const std::string& diffuse,
					   CVector3           colour   = { 1.f,1.f,.7f },
					   float              strength = 1.f,
					   CVector3           position = { 0.f,0.f,0.f },
					   CVector3           rotation = { 0.f,0.f,0.f },
					   float              scale    = 1.f);

			void SetRotation(CVector3 rotation,
							 int      node) override;
			void    LoadNewMesh(std::string newMesh) override;
			void    Render(bool basicGeometry) override;
			~ CDX12Light() override;
	};
	
	
}
