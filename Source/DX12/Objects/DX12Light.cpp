#include "DX12Light.h"

#include <utility>

CDX12Light::CDX12Light(CDX12Engine* engine,
	std::string  mesh,
	std::string  name,
	std::string diffuse,
	CVector3     colour,
	float        strength,
	CVector3     position,
	CVector3     rotation,
	float        scale)
	: CDX12GameObject(engine, std::move(mesh), std::move(name), diffuse, position, rotation, scale),
	mColour(colour),
	mStrength(strength)
{
}

void CDX12Light::SetRotation(CVector3 rotation,
	int node)
{
	CDX12GameObject::SetRotation(rotation, node);
}

void CDX12Light::Render()
{
		CDX12GameObject::Render();
}
