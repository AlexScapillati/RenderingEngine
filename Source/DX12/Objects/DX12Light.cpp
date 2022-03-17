
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

	 CDX12Light::~CDX12Light() = default;

	 CDX12DirectionalLight::CDX12DirectionalLight(CDX12Engine* engine,
			 const std::string& mesh,
			 const std::string& name,
			 const std::string& diffuse,
			 const CVector3& colour,
			 float strength,
			 const CVector3& position,
			 const CVector3& rotation,
			 float scale,
			 const int& shadowMapSize,
			 const float& width,
			 const float& height,
			 const float& nearClip,
			 const float& farClip):
		 CDX12GameObject(engine, mesh, name, diffuse, position, rotation, scale),
		 CDirectionalLight(colour, strength, shadowMapSize, width, height, nearClip, farClip)
	 {
	 }

	 void CDX12Light::SetRotation(CVector3 rotation,
								  int      node) {
		CDX12GameObject::SetRotation(rotation, node);
	}

	 void CDX12Light::LoadNewMesh(std::string newMesh) { CDX12GameObject::LoadNewMesh(newMesh); }

	 void CDX12Light::Render(bool basicGeometry) { CDX12GameObject::Render(basicGeometry); }

	 void CDX12DirectionalLight::SetRotation(CVector3 rotation,
		int      node) {
		CDX12GameObject::SetRotation(rotation, node);
	}

	 void CDX12DirectionalLight::LoadNewMesh(std::string newMesh) { CDX12GameObject::LoadNewMesh(newMesh); }

	 void CDX12DirectionalLight::Render(bool basicGeometry) { CDX12GameObject::Render(basicGeometry); }

	 void* CDX12DirectionalLight::RenderFromThis()
	 {
		 return nullptr;
	 }

	 CDX12DirectionalLight::~CDX12DirectionalLight()
	{
	}

	 void CDX12DirectionalLight::SetShadowMapSize(int s) { mShadowMapSize = s; }

	 void CDX12PointLight::SetShadowMapSize(int size) { mShadowMapSize = size; }

	 void CDX12PointLight::SetRotation(CVector3 rotation,
		 int      node) {
		 CDX12GameObject::SetRotation(rotation, node);
	 }

	 void CDX12PointLight::LoadNewMesh(std::string newMesh) { CDX12GameObject::LoadNewMesh(newMesh); }

	 void CDX12PointLight::Render(bool basicGeometry) { CDX12GameObject::Render(basicGeometry); }

	 void* CDX12PointLight::RenderFromThis()
	 {
		 return nullptr;
	 }

	 CDX12PointLight::~CDX12PointLight()
	 {
	 }

}