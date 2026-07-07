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
	
	std::atomic<double> currentSampleRate{ 48000 };


	// scope comp
	std::atomic<float> fftSmooth{ 0.8f };

	std::atomic<float> displaySmooth{ 0.15f };

	std::atomic<float> dbMin{ -100 };		//sollte int sein...


	// spec comp


	std::atomic<int> spec_dBVisible{ -65 };
	std::atomic<int> spec_dBFloor{ -80 };


	//filter

	// filter cutoff (high-pass)
	std::atomic<double> hp_cutoff{ 100 };




		

};