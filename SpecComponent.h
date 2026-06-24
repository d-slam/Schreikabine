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

		if (fifoIndex == fftSize / 16)
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

		constexpr int axisWidth = 50;

		auto specArea = getLocalBounds().withTrimmedRight(axisWidth);

		g.setOpacity(1.0f);
		g.drawImage(spectrogramImage,
			specArea.toFloat());

		g.setColour(juce::Colours::grey.withAlpha(0.3f));

		std::array<float, 10> freqs =
		{
			20.0f, 50.0f, 100.0f, 200.0f, 500.0f,
			1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f
		};

		float nyquist =
			(float)audioState.currentSampleRate.load() * 0.5f;

		for (auto freq : freqs)
		{
			if (freq > nyquist)
				continue;

			auto y = frequencyToY(freq,
				(float)specArea.getHeight());

			juce::String label;

			if (freq >= 1000.0f)
				label = juce::String(freq / 1000.0f, 0) + "k";
			else
				label = juce::String((int)freq);

			// horizontale Grid-Linie
			g.drawHorizontalLine(
				(int)y,
				0.0f,
				(float)specArea.getRight());

			// kleiner Tick an der Achse
			g.drawHorizontalLine(
				(int)y,
				(float)specArea.getRight(),
				(float)specArea.getRight() + 5.0f);

			// Frequenz-Label rechts
			g.drawText(label,
				specArea.getRight() + 6,
				(int)y - 8,
				axisWidth - 6,
				16,
				juce::Justification::centredLeft);
		}
	}

	void resized() override
	{
		constexpr int axisWidth = 50;

		spectrogramImage = juce::Image(
			juce::Image::RGB,
			juce::jmax(1, getWidth() - axisWidth),
			juce::jmax(1, getHeight()),
			true);
	}


	static constexpr auto fftOrder = 12;		//def 10
	static constexpr auto fftSize = 1 << fftOrder;

private:

	juce::dsp::WindowingFunction<float> window
	{
		fftSize,
		juce::dsp::WindowingFunction<float>::hann
	};

	juce::dsp::FFT forwardFFT;
	juce::Image spectrogramImage;
	std::array<float, fftSize> fifo{};
	std::array<float, fftSize * 2> fftData{};
	int fifoIndex = 0;
	bool nextFFTBlockReady = false;



	AudioState& audioState;

	float frequencyToY(float freq, float height)
	{
		constexpr float minFreq = 20.0f;

		float maxFreq =
			(float)audioState.currentSampleRate.load() * 0.5f;

		auto norm =
			(std::log10(freq) - std::log10(minFreq))
			/ (std::log10(maxFreq) - std::log10(minFreq));

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



	void drawNextLineOfSpectrogram()
	{
		auto rightHandEdge = spectrogramImage.getWidth() - 1;
		auto imageHeight = spectrogramImage.getHeight();

		spectrogramImage.moveImageSection(
			0, 0,
			1, 0,
			rightHandEdge,
			imageHeight);

		window.multiplyWithWindowingTable(
			fftData.data(),
			fftSize);

		forwardFFT.performFrequencyOnlyForwardTransform(
			fftData.data());

		auto maxLevel =
			juce::FloatVectorOperations::findMinAndMax(
				fftData.data(),
				fftSize / 2);

		juce::Image::BitmapData bitmap
		{
			spectrogramImage,
			rightHandEdge,
			0,
			1,
			imageHeight,
			juce::Image::BitmapData::writeOnly
		};

		constexpr float minFreq = 20.0f;

		float sampleRate =
			(float)audioState.currentSampleRate.load();

		float nyquist =
			sampleRate * 0.5f;

		for (int y = 0; y < imageHeight; ++y)
		{
			float norm =
				1.0f - (float)y / (float)(imageHeight - 1);

			float freq =
				minFreq *
				std::pow(nyquist / minFreq, norm);

			int fftDataIndex =
				juce::jlimit(
					0,
					fftSize / 2 - 1,
					(int)(freq * fftSize / sampleRate));

			float level =
				juce::jmap(
					fftData[fftDataIndex],
					0.0f,
					juce::jmax(maxLevel.getEnd(), 1e-5f),
					0.0f,
					1.0f);

			bitmap.setPixelColour(
				0,
				y,
				juce::Colour::fromHSV(
					level,
					1.0f,
					level,
					1.0f));
		}
	}

	




	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpecComponent)

};