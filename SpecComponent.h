/*
  ==============================================================================

	SpecComponent.h
	Created: 22 Jun 2026 8:20:10am
	Author:  SchoeDam

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "AudioState.h"

class SpecComponent : public juce::Component, private juce::Timer
{
public:
	SpecComponent(AudioState& state) : audioState(state), forwardFFT(fftOrder), spectrogramImage(juce::Image::RGB, 512, 512, true)
	{
			

		setSize(640, 512);
		//setOpaque(true);
		startTimerHz(60);
	}

	void pushNextSampleIntoFifo(float sample) noexcept
	{
		// if the fifo contains enough data, set a flag to say
		// that the next line should now be rendered..
		if (fifoIndex == fftSize) // [8]
		{
			if (!nextFFTBlockReady) // [9]
			{
				std::fill(fftData.begin(), fftData.end(), 0.0f);
				std::copy(fifo.begin(), fifo.end(), fftData.begin());
				nextFFTBlockReady = true;
			}
			fifoIndex = 0;
		}
		fifo[(size_t)fifoIndex++] = sample; // [9]
	}

	void paint(juce::Graphics& g) override
	{
		g.fillAll(juce::Colours::black);

		g.setOpacity(1.0f);
		g.drawImage(spectrogramImage, getLocalBounds().toFloat());
	}


	static constexpr auto fftOrder = 10;		//def 10
	static constexpr auto fftSize = 1 << fftOrder;
private:


	juce::dsp::FFT forwardFFT;
	juce::Image spectrogramImage;
	std::array<float, fftSize> fifo{};
	std::array<float, fftSize * 2> fftData{};
	int fifoIndex = 0;
	bool nextFFTBlockReady = false;

	AudioState& audioState;




	void timerCallback() override
	{
		if (nextFFTBlockReady)
		{
			drawNextLineOfSpectrogram();
			nextFFTBlockReady = false;
			repaint();
		}
	}


	void drawNextLineOfSpectrogram()
	{
		auto rightHandEdge = spectrogramImage.getWidth() - 1;
		auto imageHeight = spectrogramImage.getHeight();
		// first, shuffle our image leftwards by 1 pixel..
		spectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, imageHeight); // [1]
		// then render our FFT data..
		forwardFFT.performFrequencyOnlyForwardTransform(fftData.data()); // [2]
		// find the range of values produced, so we can scale our rendering to
		// show up the detail clearly
		auto maxLevel = juce::FloatVectorOperations::findMinAndMax(fftData.data(), fftSize / 2); // [3]
		juce::Image::BitmapData bitmap{ spectrogramImage, rightHandEdge, 0, 1, imageHeight, juce::Image::BitmapData::writeOnly }; // [4]
		for (auto y = 1; y < imageHeight; ++y) // [5]
		{
			auto skewedProportionY = 1.0f - std::exp(std::log((float)y / (float)imageHeight) * 0.2f);
			auto fftDataIndex = (size_t)juce::jlimit(0, fftSize / 2, (int)(skewedProportionY * fftSize / 2));
			auto level = juce::jmap(fftData[fftDataIndex], 0.0f, juce::jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);
			bitmap.setPixelColour(0, y, juce::Colour::fromHSV(level, 1.0f, level, 1.0f)); // [6]
		}
	}






	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpecComponent)

};