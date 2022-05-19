#pragma once

#include "DX12GameObject.h"
#include "../../Common/CLight.h"

namespace DX12
{
	class CDX12Light :virtual public CDX12GameObject, virtual public CLight
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
							 int      node = 0) override;
			void    Render(bool basicGeometry = false) override;
	};
	
	
}
