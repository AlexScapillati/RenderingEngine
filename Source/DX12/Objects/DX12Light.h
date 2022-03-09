#pragma once

#include "DX12GameObject.h"

#include "../../Common/CLight.h"

#include "..\..\Math/CVector3.h"

namespace DX12
{
	class CDX12Light final : public CDX12GameObject, public CLight
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
					   float              scale    = 1.f)
				:
				CDX12GameObject(engine, mesh, name, diffuse, position, rotation, scale),
				CLight(colour, strength)
				{
				}

			void SetRotation(CVector3 rotation,
							 int      node) override;
			void    LoadNewMesh(std::string newMesh) override;
			void    Render(bool basicGeometry) override;
			~ CDX12Light() override = default ;
	};
	
	class CDX12DirectionalLight : virtual public CDX12GameObject, virtual public CDirectionalLight
	{
		public :
			CDX12DirectionalLight(CDX12Engine*       engine,
								  const std::string& mesh,
								  const std::string& name,
								  const std::string& diffuse,
								  const CVector3&    colour,
								  float              strength,
								  const CVector3&    position,
								  const CVector3&    rotation,
								  float              scale,
								  const int&         shadowMapSize = 2048,
								  const float&       width = 1000,
								  const float&       height = 1000,
								  const float&       nearClip = 0.001f,
								  const float&       farClip = 1000)
				:
				CDX12GameObject(engine, mesh, name, diffuse, position, rotation, scale),
				CDirectionalLight(colour, strength, shadowMapSize, width, height, nearClip, farClip)
				{
				}

			inline void SetRotation(CVector3 rotation,
									int      node) override;
			inline void LoadNewMesh(std::string newMesh) override;
			inline void Render(bool basicGeometry) override;
			~ CDX12DirectionalLight() override;
			void SetShadowMapSize(int s) override;
	};

	class CDX12PointLight final : virtual public CDX12GameObject, virtual public CPointLight
	{
		public :
			CDX12PointLight(CDX12Engine*       engine,
							const std::string& mesh,
							const std::string& name,
							const std::string& diffuse,
							const CVector3&    colour,
							float              strength,
							const CVector3&    position,
							const CVector3&    rotation,
							float              scale,
							const int&         shadowMapSize = 2048)
				:
				CDX12GameObject(engine, mesh, name, diffuse, position, rotation, scale),
				CPointLight(colour, strength, shadowMapSize)
				{
				}

			void SetRotation(CVector3 rotation,
							 int      node) override;
			void LoadNewMesh(std::string newMesh) override;
			void Render(bool basicGeometry) override;
			~ CDX12PointLight() override;
			void SetShadowMapSize(int size) override;
	};

	class CDX12SpotLight final : virtual public CDX12GameObject, virtual public CSpotLight
	{
		public :
			CDX12SpotLight(CDX12Engine*       engine,
						   const std::string& mesh,
						   const std::string& name,
						   const std::string& diffuse,
						   const CVector3&    colour,
						   float              strength,
						   const CVector3&    position,
						   const CVector3&    rotation,
						   float              scale,
						   const int&         shadowMapSize = 2048,
						   const float&       coneAngle = 90.f)
				:
				CDX12GameObject(engine, mesh, name, diffuse, position, rotation, scale),
				CSpotLight(colour, strength, shadowMapSize, coneAngle)
				{
				}

			void SetRotation(CVector3 rotation,
							 int      node) override;
			void LoadNewMesh(std::string newMesh) override;
			void Render(bool basicGeometry) override;
			~ CDX12SpotLight() override;
			void SetConeAngle(float value) override;
			void SetShadowMapsSize(int value) override;
	};

	inline void CDX12Light::SetRotation(CVector3 rotation,
										int      node) { CDX12GameObject::SetRotation(rotation, node); }

	inline void CDX12Light::LoadNewMesh(std::string newMesh) { CDX12GameObject::LoadNewMesh(newMesh); }

	inline void CDX12Light::Render(bool basicGeometry) { CDX12GameObject::Render(basicGeometry); }

	inline void CDX12DirectionalLight::SetRotation(CVector3 rotation,
												   int      node) { CDX12GameObject::SetRotation(rotation, node); }

	inline void CDX12DirectionalLight::LoadNewMesh(std::string newMesh) { CDX12GameObject::LoadNewMesh(newMesh); }

	inline void CDX12DirectionalLight::Render(bool basicGeometry) { CDX12GameObject::Render(basicGeometry); }

	inline CDX12DirectionalLight::~ CDX12DirectionalLight()
		{
		}

	inline void CDX12DirectionalLight::SetShadowMapSize(int s) { mShadowMapSize = s; }

	inline void CDX12PointLight::SetShadowMapSize(int size) { mShadowMapSize = size; }

	inline void CDX12SpotLight::SetConeAngle(float value) { mConeAngle = value; }

	inline void CDX12SpotLight::SetShadowMapsSize(int value) { mShadowMapSize = value; }

	inline void CDX12PointLight::SetRotation(CVector3 rotation,
											 int      node) {CDX12GameObject::SetRotation(rotation, node); }

	inline void CDX12PointLight::LoadNewMesh(std::string newMesh) { CDX12GameObject::LoadNewMesh(newMesh); }

	inline void CDX12PointLight::Render(bool basicGeometry) { CDX12GameObject::Render(basicGeometry); }

	inline CDX12PointLight::~ CDX12PointLight()
		{
		}

	inline void CDX12SpotLight::SetRotation(CVector3 rotation,
											int      node) { CGameObject::SetRotation(rotation, node); }

	inline void CDX12SpotLight::LoadNewMesh(std::string newMesh) { CDX12GameObject::LoadNewMesh(newMesh); }

	inline void CDX12SpotLight::Render(bool basicGeometry) { CDX12GameObject::Render(basicGeometry); }

	inline CDX12SpotLight::~ CDX12SpotLight()
		{
		}
}
