//--------------------------------------------------------------------------------------
// Timer class - works like a stopwatch
//--------------------------------------------------------------------------------------
#pragma once


#include "stdint.h"

class Timer
{
public:

	// Constructor //

	Timer();

	
	// Timer control //

	// Start the timer running
	void Start();

	// Stop the timer running
	void Stop();

	// Reset the timer to zero
	void Reset();


	// Timing //

	// Get frequency of the timer being used (in counts per second)
	float GetFrequency() const;

	// Get time passed (seconds) since since timer was started or last reset
	float GetTime() const;

	// Get time passed (seconds) since last call to this function. If this is the first call, then
	// the time since timer was started or the last reset is returned
	float GetLapTime();


private:
	// Is the timer running
	bool mRunning;


	// Using high resolution timer and if so its frequency
	bool     mHighRes;
	uint64_t mHighResFreq;

	// Start time and last lap start time of high-resolution timer
	uint64_t mHighResStart;
	uint64_t mHighResLap;

	// Time when high-resolution timer was stopped (if it has been)
	uint64_t mHighResStop;


	// Start time and last lap start time of low-resolution timer
	uint32_t mLowResStart;
	uint32_t mLowResLap;

	// Time when low-resolution timer was stopped (if it has been)
	uint32_t mLowResStop;
};