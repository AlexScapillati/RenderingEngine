#pragma once

#include "DX12GameObject.h"
#include "..\..\Math/CVector3.h"

namespace DX12
{
	class CDX12Light : public CDX12GameObject, public CLight
	{
		public:
			CDX12Light(CDX12Engine*       engine,
					   const std::string  mesh,
					   const std::string  name,
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
			virtual ~ CDX12Light() = default ;
	};

	class CDX12DirectionalLight : public CDX12Light, public CDirectionalLight
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
								  const CVector3&    col,
								  const float&       s,
								  const int&         shadowMapSize,
								  const float&       width,
								  const float&       height,
								  const float&       nearClip,
								  const float&       farClip)
				:
				CDX12Light(engine, mesh, name, diffuse, colour, strength, position, rotation, scale),
				CDirectionalLight(col, s, shadowMapSize, width, height, nearClip, farClip)
				{
				}

			inline void SetRotation(CVector3 rotation,
									int      node) override;
			inline void LoadNewMesh(std::string newMesh) override;
			inline void Render(bool basicGeometry) override;
			~ CDX12DirectionalLight() override;
			void SetShadowMapSize(int s) override;
	};

	class CDX12PointLight final : public CDX12Light, CPointLight
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
							const CVector3&    col,
							const float&       s,
							const int&         shadowMapSize)
				:
				CDX12Light(engine, mesh, name, diffuse, colour, strength, position, rotation, scale),
				CPointLight(col, s, shadowMapSize)
				{
				}

			void SetRotation(CVector3 rotation,
							 int      node) override;
			void LoadNewMesh(std::string newMesh) override;
			void Render(bool basicGeometry) override;
			~ CDX12PointLight() override;
			void SetShadowMapSize(int size) override;
	};

	class CDX12SpotLight final : public CDX12Light, CSpotLight
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
						   const CVector3&    col,
						   const float&       s,
						   const int&         shadowMapSize,
						   const float&       coneAngle)
				:
				CDX12Light(engine, mesh, name, diffuse, colour, strength, position, rotation, scale),
				CSpotLight(col, s, shadowMapSize, coneAngle)
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
												   int      node) { CDX12Light::SetRotation(rotation, node); }

	inline void CDX12DirectionalLight::LoadNewMesh(std::string newMesh) { CDX12Light::LoadNewMesh(newMesh); }

	inline void CDX12DirectionalLight::Render(bool basicGeometry) { CDX12Light::Render(basicGeometry); }

	inline CDX12DirectionalLight::~ CDX12DirectionalLight()
		{
		}

	inline void CDX12DirectionalLight::SetShadowMapSize(int s) { mShadowMapSize = s; }

	inline void CDX12PointLight::SetShadowMapSize(int size) { mShadowMapSize = size; }

	inline void CDX12SpotLight::SetConeAngle(float value) { mConeAngle = value; }

	inline void CDX12SpotLight::SetShadowMapsSize(int value) { mShadowMapSize = value; }

	inline void CDX12PointLight::SetRotation(CVector3 rotation,
											 int      node) { CDX12Light::SetRotation(rotation, node); }

	inline void CDX12PointLight::LoadNewMesh(std::string newMesh) { CDX12Light::LoadNewMesh(newMesh); }

	inline void CDX12PointLight::Render(bool basicGeometry) { CDX12Light::Render(basicGeometry); }

	inline CDX12PointLight::~ CDX12PointLight()
		{
		}

	inline void CDX12SpotLight::SetRotation(CVector3 rotation,
											int      node) { CDX12Light::SetRotation(rotation, node); }

	inline void CDX12SpotLight::LoadNewMesh(std::string newMesh) { CDX12Light::LoadNewMesh(newMesh); }

	inline void CDX12SpotLight::Render(bool basicGeometry) { CDX12Light::Render(basicGeometry); }

	inline CDX12SpotLight::~ CDX12SpotLight()
		{
		}
}
