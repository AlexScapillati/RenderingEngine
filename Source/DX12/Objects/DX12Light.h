#pragma once

#include "DX12GameObject.h"

class CDX12Light : public CDX12GameObject
{
public:

	CDX12Light(CDX12Engine* engine,
		std::string  mesh,
		std::string  name,
		std::string diffuse,
		CVector3     colour = { 1.f,1.f,.7f },
		float        strength = 1.f,
		CVector3     position = { 0.f,0.f,0.f },
		CVector3     rotation = { 0.f,0.f,0.f },
		float        scale = 1.f);

	void SetRotation(CVector3 rotation,
		int      node) override;

	void Render() override;

	// Setters and Getters

	void     SetColour(CVector3 c) { mColour = c; }
	void     SetStrength(float s) { mStrength = s; }
	CVector3 Colour() const { return mColour; }
	float    Strength() const { return mStrength; }


private:
	CVector3 mColour;
	float    mStrength;
};

