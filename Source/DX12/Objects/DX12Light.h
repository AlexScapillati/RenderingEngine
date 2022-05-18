#pragma once

#include "DX12GameObject.h"

namespace DX12
{
	class CDX12Light : public CDX12GameObject
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

		void      SetColour(CVector3 colour) { mColour = colour; }
		void      SetStrength(float strength) { mStrength = strength; }
		CVector3& GetColour() { return mColour; }
		float&    GetStrength() { return mStrength; }

	protected:
		CVector3 mColour;
		float    mStrength;
	};
	
	
}
