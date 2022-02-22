#pragma once

#include "../Math/CVector2.h"

class CScene;
class CGameObject;
class IEngine;

class CGui
{
public:
		virtual ~CGui() = default;

		CGui(IEngine* engine);

	// Disable copy/assignment/default constructors
	CGui() = delete;
	CGui(const CGui&) = delete;
	CGui(const CGui&&) = delete;
	CGui& operator=(const CGui&) = delete;
	CGui& operator=(const CGui&&) = delete;

	virtual void Begin(float& frameTime);
	virtual void Show(float& frameTime);
	virtual void AddObjectsMenu() const;
	virtual void DisplayPropertiesWindow() const;
	virtual void DisplayObjects();
	virtual void DisplaySceneSettings(bool& b) const;
	virtual void DisplayShadowMaps() const;
	virtual void End() const;

	bool IsSceneFullscreen() const { return mViewportFullscreen; }

private:
	IEngine* mEngine = nullptr;
	CScene* mScene = nullptr;
	CGameObject* mSelectedObj = nullptr;
	bool             mViewportFullscreen = false;
	CVector2         mViewportWindowPos = {};
};
