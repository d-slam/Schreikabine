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
		: audioState(state),
		forwardFFT(fftOrder),
		window(fftSize, juce::dsp::WindowingFunction<float>::hann),
		spectrogramImage(juce::Image::RGB, 512, 512, true)
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

		g.setImageResamplingQuality(
			juce::Graphics::highResamplingQuality);


		constexpr int axisWidth = 55;

		auto specArea =
			getLocalBounds()
			.withTrimmedRight(axisWidth);


		g.drawImage(
			spectrogramImage,
			specArea.toFloat());


		drawFrequencyAxis(g, specArea);
	}



	void resized() override
	{
		constexpr int axisWidth = 55;

		spectrogramImage =
			juce::Image(
				juce::Image::RGB,
				juce::jmax(1, getWidth() - axisWidth),
				juce::jmax(1, getHeight()),
				true);
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



	float frequencyToY(float freq, float height)
	{
		auto norm =
			(std::log10(freq) - std::log10(minFreq))
			/
			(std::log10(maxFreq) - std::log10(minFreq));


		return height * (1.0f - norm);
	}



	void drawFrequencyAxis(
		juce::Graphics& g,
		juce::Rectangle<int> area)
	{

		g.setColour(
			juce::Colours::grey.withAlpha(0.35f));


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

			auto y =
				frequencyToY(
					freq,
					(float)area.getHeight());


			g.drawHorizontalLine(
				(int)y,
				0,
				area.getRight());



			juce::String label;


			if (freq >= 1000)
				label =
				juce::String(freq / 1000, 0)
				+ "k";
			else
				label =
				juce::String((int)freq);



			g.drawText(
				label,
				area.getRight() + 5,
				(int)y - 8,
				45,
				16,
				juce::Justification::centredLeft);
		}
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

		auto right =
			spectrogramImage.getWidth() - 1;


		auto height =
			spectrogramImage.getHeight();



		spectrogramImage.moveImageSection(
			0, 0,
			1, 0,
			right,
			height);



		window.multiplyWithWindowingTable(
			fftData.data(),
			fftSize);



		forwardFFT.performFrequencyOnlyForwardTransform(
			fftData.data());



		juce::Image::BitmapData bitmap
		{
			spectrogramImage,
			right,
			0,
			1,
			height,
			juce::Image::BitmapData::writeOnly
		};



		float sampleRate =
			(float)audioState.currentSampleRate.load();




		for (int y = 0; y < height; y++)
		{


			float norm =
				1.0f -
				(float)y /
				(float)(height - 1);



			float freq =
				minFreq *
				std::pow(
					maxFreq / minFreq,
					norm);




			float binFloat =
				freq *
				fftSize /
				sampleRate;



			int b0 =
				juce::jlimit(
					0,
					fftSize / 2 - 1,
					(int)binFloat);


			int b1 =
				juce::jmin(
					b0 + 1,
					fftSize / 2 - 1);



			float frac =
				binFloat -
				(float)b0;



			float magnitude =
				fftData[b0] +
				frac *
				(fftData[b1] - fftData[b0]);



			float db =
				juce::Decibels::gainToDecibels(
					magnitude + 1e-9f);




			float level =
				juce::jlimit(
					0.0f,
					1.0f,
					(db + 75.0f)
					/ 75.0f);



			// Gamma
			level =
				std::pow(
					level,
					0.55f);




			// RX / moderne Palette
			auto colour =
				juce::Colour::fromFloatRGBA(
					level * 0.15f,
					level * 0.85f,
					level,
					1.0f);



			bitmap.setPixelColour(
				0,
				y,
				colour);
		}
	}



	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpecComponent)
};