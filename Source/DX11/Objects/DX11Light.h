#pragma once


#include "DX11GameObject.h"
#include "../../Common/CLight.h"

/*

Normal point light class
Does not cast shadows

*/

namespace DX11
{
	class CDX11Light : public CLight, public CDX11GameObject
	{
	public:

		CDX11Light(CDX11Engine*       engine,
				   const std::string& mesh,
				   const std::string& name,
				   const std::string& diffuse,
				   const CVector3&    colour   = { 0.0f,0.0f,0.0f },
				   const float&       strength = 0.0f,
				   const CVector3&    position = { 0,0,0 },
				   const CVector3&    rotation = { 0,0,0 },
				   const float&       scale    = 1);

		void Render(bool basicGeometry = false) override;
		void LoadNewMesh(std::string newMesh) override;
		~CDX11Light() override;
	};
}