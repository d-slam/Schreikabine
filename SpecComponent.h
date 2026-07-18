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

	SpecComponent(AudioState& state) 
		: audioState(state), forwardFFT(fftOrder), window(fftSize, juce::dsp::WindowingFunction<float>::hann), spectrogramImage(juce::Image::RGB, 512, 512, true)
	{
		setSize(640, 512);
		startTimerHz(60);
	}


	void pushNextSampleIntoFifo(float sample) noexcept
	{
		if (fifoIndex == fftSize / 8)
		{
			if (!nextFFTBlockReady)
			{
				std::fill(fftData.begin(), fftData.end(), 0.0f);
				std::copy(fifo.begin(), fifo.end(), fftData.begin());
				nextFFTBlockReady = true;
			}
			fifoIndex = 0;
		}
		fifo[(size_t)fifoIndex++] = sample;
	}


	void paint(juce::Graphics& g) override
	{
		g.fillAll(juce::Colours::black);

		g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);

		constexpr int axisWidth = 55;

		auto specArea = getLocalBounds().withTrimmedRight(axisWidth);

		g.drawImage(spectrogramImage, specArea.toFloat());

		drawFrequencyAxis(g, specArea);
	}



	void resized() override
	{
		constexpr int axisWidth = 55;
		spectrogramImage = juce::Image(juce::Image::RGB, juce::jmax(1, getWidth() - axisWidth), juce::jmax(1, getHeight()), true);
	}



	static constexpr auto fftOrder = 13;
	static constexpr auto fftSize = 1 << fftOrder;



private:

	juce::dsp::FFT forwardFFT;

	juce::dsp::WindowingFunction<float> window
	{
		fftSize,
		juce::dsp::WindowingFunction<float>::hann
	};

	juce::Image spectrogramImage;

	std::array<float, fftSize> fifo{};
	std::array<float, fftSize * 2> fftData{};

	int fifoIndex = 0;
	bool nextFFTBlockReady = false;

	AudioState& audioState;

	static constexpr float minFreq = 50.0f;
	static constexpr float maxFreq = 10000.0f;

	void drawNextLineOfSpectrogram()
	{

		auto right = spectrogramImage.getWidth() - 1;

		auto height = spectrogramImage.getHeight();

		spectrogramImage.moveImageSection(0, 0, 1, 0, right, height);

		window.multiplyWithWindowingTable(fftData.data(), fftSize);

		forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());

		juce::Image::BitmapData bitmap
		{
			spectrogramImage,
			right,
			0,
			1,
			height,
			juce::Image::BitmapData::writeOnly
		};

		float sampleRate = (float)audioState.currentSampleRate.load();

		for (int y = 0; y < height; y++)
		{

			float norm = 1.0f - (float)y / (float)(height - 1);

			float freq = minFreq * std::pow(maxFreq / minFreq, norm);

			float binFloat = freq * fftSize / sampleRate;

			int b0 = juce::jlimit(0, fftSize / 2 - 1, (int)binFloat);

			int b1 = juce::jmin(b0 + 1, fftSize / 2 - 1);

			float frac = binFloat - (float)b0;

			float magnitude = (fftData[b0] + frac * (fftData[b1] - fftData[b0])) / 8.0f;	// def: 128.0f  teilnormierung für hell dunkel 

			float db = juce::Decibels::gainToDecibels(magnitude + 1e-9f);

			auto visibleDb = -12.0f;
			auto floorDb = -36.0f;
			//auto visibleDb = (float)audioState.spec_dBVisible.load();
			//auto floorDb = (float)audioState.spec_dBFloor.load();

			float level = juce::jmap(db, floorDb, 0.0f, 0.0f, 1.0f);

			level = juce::jlimit(0.0f, 1.0f, level);

			if (db < visibleDb)
			{
				float fade = juce::jmap(db, floorDb, visibleDb, 0.0f, 1.0f);
				level *= fade;
			}



			bitmap.setPixelColour(0, y, rxColourMap(level));
		}
	}

	static juce::Colour rxColourMap(float x)
	{
		x = juce::jlimit(0.0f, 1.0f, x);

		struct Stop { float pos;	juce::Colour colour; };
		static const Stop stops[] =
		{
			{ 0.00f, juce::Colour(0xff000000) }, // schwarz
			{ 0.20f, juce::Colour(0xff001040) }, // dunkelblau
			{ 0.40f, juce::Colour(0xff401080) }, // violett
			{ 0.65f, juce::Colour(0xffd06000) }, // orange
			{ 0.85f, juce::Colour(0xffffff00) }, // gelb
			{ 1.00f, juce::Colours::white }
		};

		for (int i = 0; i < 5; ++i)
		{
			if (x <= stops[i + 1].pos)
			{
				float t = (x - stops[i].pos) / (stops[i + 1].pos - stops[i].pos);

				return stops[i].colour.interpolatedWith(stops[i + 1].colour, t);
			}
		}
		return juce::Colours::white;
	}

	void drawFrequencyAxis(juce::Graphics& g, juce::Rectangle<int> area)
	{
		g.setColour(juce::Colours::grey.withAlpha(0.5f));

		std::array<float, 8> freqs =
		{
			50,
			100,
			250,
			500,
			1000,
			2000,
			5000,
			10000
		};

		for (auto freq : freqs)
		{
			auto y = frequencyToY(freq, (float)area.getHeight());
			g.drawHorizontalLine((int)y, 0, area.getRight());
			juce::String label;

			if (freq >= 1000)	label = juce::String(freq / 1000, 0) + "k";
			else				label = juce::String((int)freq);

			g.drawText(label, area.getRight() + 5, (int)y - 8, 45, 16, juce::Justification::centredLeft);
		}
	}

	float frequencyToY(float freq, float height)
	{
		auto norm = (std::log10(freq) - std::log10(minFreq)) / (std::log10(maxFreq) - std::log10(minFreq));
		return height * (1.0f - norm);
	}

	void timerCallback() override
	{
		if (nextFFTBlockReady)
		{
			drawNextLineOfSpectrogram();
			nextFFTBlockReady = false;
			repaint();
		}
	}




	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpecComponent)
};