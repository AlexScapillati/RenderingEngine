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
	
	class CDX12DirectionalLight :  public CDX12GameObject,  public CDirectionalLight
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
								  const float&       width         = 1000,
								  const float&       height        = 1000,
								  const float&       nearClip      = 0.001f,
								  const float&       farClip       = 1000);

			inline void SetRotation(CVector3 rotation,
									int      node) override;
			inline void LoadNewMesh(std::string newMesh) override;
			inline void Render(bool basicGeometry) override;
			void* RenderFromThis() override;
			~ CDX12DirectionalLight() override;
			void SetShadowMapSize(int s) override;
	};

	class CDX12PointLight final :  public CDX12GameObject,  public CPointLight
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
			void* RenderFromThis() override;

			~ CDX12PointLight() override;
			void SetShadowMapSize(int size) override;
			
	};

}
