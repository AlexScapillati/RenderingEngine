
#include "DX12Light.h"

namespace DX12
{

	 CDX12Light::CDX12Light(CDX12Engine* engine,
		const std::string& mesh,
		const std::string& name,
		const std::string& diffuse,
		CVector3           colour,
		float              strength,
		CVector3           position,
		CVector3           rotation,
		float              scale) :
		CDX12GameObject(engine, mesh, name, diffuse, position, rotation, scale),
		CLight(colour, strength)
	{
	}

	 void CDX12Light::SetRotation(CVector3 rotation, int node)
	 {
		 CDX12GameObject::SetRotation(rotation, node);
	 }

	void CDX12Light::LoadNewMesh(std::string newMesh) { CDX12GameObject::LoadNewMesh(newMesh); }

	void CDX12Light::Render(bool basicGeometry) { CDX12GameObject::Render(basicGeometry); }
	 CDX12Light::~CDX12Light() = default;
}