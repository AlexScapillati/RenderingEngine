#pragma once

#include <list>
#include <string>

//********************
// Available post-processes

enum class PostProcess
{
	None,
	Copy,
	Tint,
	GreyNoise,
	Burn,
	Distort,
	Spiral,
	HeatHaze,
	ChromaticAberration,
	GaussionBlur,
	SSAO,
	Bloom,
	GodRays
};

inline std::string mPostProcessStrings[13] =
{
	"None",
	"Copy",
	"Tint",
	"GreyNoise",
	"Burn",
	"Distort",
	"Spiral",
	"HeatHaze",
	"ChromaticAberration",
	"GaussionBlur",
	"SSAO",
	"Bloom",
	"GodRays"
};

enum class PostProcessMode
{
	Fullscreen,
	Area,
	Polygon,
};

inline std::string mPostProcessModeStrings[3] =
{
	"FullScreen",
	"Area",
	"Polygon"
};

//********************

struct PostProcessFilter
{
	PostProcess type = PostProcess::None;
	PostProcessMode mode = PostProcessMode::Fullscreen;
};

inline std::list<PostProcessFilter> mPostProcessingFilters;