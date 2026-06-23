/*
  ==============================================================================

	AudioState.h
	Created: 19 Jun 2026 10:21:45am
	Author:  SchoeDam

  ==============================================================================
*/

#pragma once


#include <atomic>

struct AudioState
{
	std::atomic<double> currentSampleRate{};


	// scope comp
	std::atomic<float> fftSmooth{ 0.8f };

	std::atomic<float> displaySmooth{ 0.15f };

	std::atomic<int> dbMin{ -100 };


	// spec comp




};