#pragma once

#include "CGameObject.h"
#include "../Math/CVector3.h"

class CLight : virtual public CGameObject
{
	public:
		~CLight() override = default;

	CLight(const CVector3& col = {1,1,1}, const float& s = 1000) : mColour(col), mStrength(s) {}

		void      SetColour(CVector3 colour) { mColour = colour; }
		void      SetStrength(float strength) { mStrength = strength; }
		CVector3& GetColour() { return mColour; }
		float&    GetStrength() { return mStrength; }
		virtual void Render(bool basicGeometry = false) override = 0;
		virtual void LoadNewMesh(std::string newMesh) override = 0;

	protected:
		CVector3 mColour;
		float    mStrength  =1;
};

class CSpotLight : public CLight
{
	public:

		CSpotLight(const CVector3& col = {1,1,1}, const float& s = 1000.f, const int& shadowMapSize = 2048, const float& coneAngle = 90.f)
			: CLight(col, s), mShadowMapSize(shadowMapSize), mConeAngle(coneAngle) {}


		virtual void SetConeAngle(float value) = 0;
		virtual void SetShadowMapsSize(int value) = 0;

		int&   GetShadowMapSize() { return mShadowMapSize; }
		float& GetConeAngle() { return mConeAngle; }

		virtual void   Render(bool basicGeometry = false) override = 0;
		virtual void   LoadNewMesh(std::string newMesh) override = 0;

	protected:
		int   mShadowMapSize = 0;
		float mConeAngle = 0;
};


class CDirectionalLight :  public CLight
{
	public:

		CDirectionalLight(const CVector3& col = {1,1,1},
			const float& s = 100.f,
			const int& shadowMapSize = 2048, 
			const float& width = 1000.f,
			const float& height = 1000.f,
			const float& nearClip =0.0001f,
			const float& farClip = 1000.f)
			: CLight(col, s),
	mShadowMapSize(shadowMapSize),
	mWidth(width),
	mHeight(height),
	mNearClip(nearClip),
	mFarClip(farClip){}

		virtual void SetShadowMapSize(int s) = 0;
		virtual void   Render(bool basicGeometry = false) override = 0;
		virtual void   LoadNewMesh(std::string newMesh) override = 0;

		auto GetNearClip() const { return mNearClip; }
		auto GetFarClip() const { return mFarClip; }
		auto SetNearClip(float n) { mNearClip = n; }
		auto SetFarClip(float n) { mFarClip = n; }
		auto GetWidth() const { return mWidth; }
		auto GetHeight() const { return mHeight; }
		auto SetWidth(float n) { mWidth = n; }
		auto SetHeight(float n) { mHeight = n; }
		auto GetShadowMapSize() const { return mShadowMapSize; }


	protected:
		int   mShadowMapSize;
		float mWidth        ;
		float mHeight       ;
		float mNearClip     ;
		float mFarClip      ;
};


class CPointLight :  public CLight
{
	public:


		CPointLight(const CVector3& col, const float& s, const int& shadowMapSize)
			: CLight(col, s), mShadowMapSize(shadowMapSize) {}

		virtual void SetShadowMapSize(int size) = 0;
		virtual void   Render(bool basicGeometry = false) override = 0;
		virtual void   LoadNewMesh(std::string newMesh) override = 0;

		int GetShadowMapSize() const { return mShadowMapSize; }

		float mSides[6][3] = {          // Starting from facing down the +ve Z direction, left handed rotations
				{ 0.0f,	 0.5f,	0.0f},  // +ve X direction (values multiplied by PI)
				{ 0.0f, -0.5f,	0.0f},  // -ve X direction
				{-0.5f,	 0.0f,	0.0f},  // +ve Y direction
				{ 0.5f,	 0.0f,	0.0f},  // -ve Y direction
				{ 0.0f,	 0.0f,	0.0f},  // +ve Z direction
				{ 0.0f,	 1.0f,  0.0f}   // -ve Z direction
		};

	protected:

		int mShadowMapSize = 0;

};
