/*
  ==============================================================================

	ScopeComponent.h
	Created: 19 Jun 2026 9:38:18am
	Author:  SchoeDam

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "AudioState.h"

class ScopeComponent : public juce::Component, private juce::Timer
{
public:
	ScopeComponent(AudioState& state) : audioState(state), forwardFFT(fftOrder), window(fftSize, juce::dsp::WindowingFunction<float>::hann)
	{
		juce::zeromem(fifo, sizeof(fifo));
		juce::zeromem(fftData, sizeof(fftData));
		//juce::zeromem(scopeData, sizeof(scopeData));
		//juce::zeromem(fftSmoothed, sizeof(fftSmoothed));

		rebuildFFTLookup();

		setOpaque(true);
		startTimerHz(30);
	}

	// wichter optimisierungscheis!!! mit steigender fftOrder brauchen wir lange bis fifo voll ist, deswegen öffters rendern mit /8 für 14te zB
	void pushNextSampleIntoFifo(float sample) noexcept
	{
		if (fifoIndex == fftSize / 8)
		{
			if (!nextFFTBlockReady)
			{
				juce::zeromem(fftData, sizeof(fftData));
				memcpy(fftData, fifo, sizeof(fifo));
				nextFFTBlockReady = true;
			}

			fifoIndex = 0;
		}
		fifo[fifoIndex++] = sample;
	}

	void rebuildFFTLookup()
	{
		float nyquist =	audioState.currentSampleRate.load() * 0.5f;

		constexpr float minFreq = 20.0f;

		const auto numPoints = fftLookup.size();

		if (numPoints == 0)	return;

		for (size_t i = 0; i < numPoints; ++i)
		{
			float proportion = static_cast<float>(i) / static_cast<float>(numPoints - 1);

			float freq = minFreq * std::pow(nyquist / minFreq, proportion);

			float bin = freq * fftSize / audioState.currentSampleRate.load();

			int bin0 = static_cast<int>(bin);
			int bin1 = juce::jmin(bin0 + 1, fftSize / 2);

			fftLookup[i] = { bin0,	bin1,	bin - static_cast<float>(bin0) };
		}
	}

private:

	static constexpr int fftOrder = 14;		//def 11
	static constexpr int fftSize = 1 << fftOrder;
	//static constexpr int scopeSize = 512;	//def 512	Obsolete da mir die size aus der component getWidth() ableiten


	juce::dsp::FFT forwardFFT;
	juce::dsp::WindowingFunction<float> window;

	float fifo[fftSize];

	float fftData[2 * fftSize];

	//float scopeData[scopeSize];
	std::vector<float> scopeData;

	//float fftSmoothed[scopeSize];
	std::vector<float> fftSmoothed;

	int fifoIndex = 0;
	bool nextFFTBlockReady = false;

	//struct FFTLookup { int index0;		int index1;		float frac; };
	struct FFTLookup { int bin0;		int bin1;		float interp; };
	std::vector<FFTLookup> fftLookup;

	AudioState& audioState;


	////////////////////////////////////////////////////////	


	float frequencyToX(float freq, float width)
	{
		constexpr float minFreq = 20.0f;			//def 20.0f
		constexpr float maxFreq = 20000.0f;			//def 20000.0f

		auto norm = (std::log10(freq) - std::log10(minFreq)) / (std::log10(maxFreq) - std::log10(minFreq));

		return norm * width;
	}


	void drawNextFrameOfSpectrum()
	{
		window.multiplyWithWindowingTable(fftData, fftSize);

		//auto start = juce::Time::getMillisecondCounterHiRes();
		forwardFFT.performFrequencyOnlyForwardTransform(fftData);
		//auto end = juce::Time::getMillisecondCounterHiRes();
		//DBG("FFT: " << (end - start) << " ms");


		auto mindB = (float)audioState.dbMin.load();
		auto maxdB = 0.0f;

		constexpr float minFreq = 20.0f;
		float nyquist = (float)audioState.currentSampleRate.load() * 0.5f;			//current sample rate

		//for (int i = 0; i < scopeSize; ++i)
		for (size_t i = 0; i < scopeData.size(); ++i)
		{
			const auto& l = fftLookup[i];

			float fftValue = fftData[l.bin0] + l.interp * (fftData[l.bin1] - fftData[l.bin0]);

			fftSmoothed[i] = fftSmoothed[i] * audioState.fftSmooth.load() + fftValue * (1 - audioState.fftSmooth.load());

			float level = juce::jmap(
				juce::jlimit(mindB, maxdB, juce::Decibels::gainToDecibels(fftSmoothed[i] + 1e-6f) - juce::Decibels::gainToDecibels((float)fftSize)),
				mindB,
				maxdB,
				0.0f,
				1.0f);

			scopeData[i] = scopeData[i] + audioState.displaySmooth.load() * (level - scopeData[i]);

		}
	}

	void timerCallback() override
	{
		if (nextFFTBlockReady)
		{
			drawNextFrameOfSpectrum();
			nextFFTBlockReady = false;
			repaint();
		}
	}


	void paint(juce::Graphics& g) override
	{

		g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
		juce::PathStrokeType stroke(1.6f,
			juce::PathStrokeType::curved,
			juce::PathStrokeType::rounded);


		g.fillAll(juce::Colours::black);

		g.setColour(juce::Colours::lime);

		auto bounds = getLocalBounds().toFloat();

		juce::Path spectrumPath;

		spectrumPath.startNewSubPath(0.0f, bounds.getBottom());

		for (size_t i = 1; i < scopeData.size(); ++i)
		{
			auto x = juce::jmap<float>(
				static_cast<float>(i),
				0.0f,
				static_cast<float>(scopeData.size() - 1),
				0.0f,
				bounds.getWidth());

			auto y = juce::jmap<float>(
				scopeData[i],
				0.0f,
				1.0f,
				bounds.getBottom(),
				bounds.getY());

			spectrumPath.lineTo(x, y);
		}


		// fabfilter style pfad
		g.setColour(juce::Colours::lime.withAlpha(0.2f));
		g.fillPath(spectrumPath);

		g.setColour(juce::Colours::lime.withAlpha(0.9f));
		g.strokePath(spectrumPath, juce::PathStrokeType(1.5f));


		//soft glow
		//g.setColour(juce::Colours::lime.withAlpha(0.08f));
		//g.strokePath(spectrumPath, juce::PathStrokeType(6.0f));

		//g.setColour(juce::Colours::lime.withAlpha(0.4f));
		//g.strokePath(spectrumPath, juce::PathStrokeType(3.0f));

		//g.setColour(juce::Colours::lime.withAlpha(1.0f));
		//g.strokePath(spectrumPath, juce::PathStrokeType(1.2f));

		//draw frequ achse

		g.setColour(juce::Colours::grey);

		std::array<float, 10> freqs =
		{
			20.0f, 50.0f, 100.0f, 200.0f, 500.0f,
			1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f
		};

		for (auto freq : freqs)
		{
			auto x = frequencyToX(freq, bounds.getWidth());

			// Tick
			g.drawVerticalLine((int)x, bounds.getBottom() - 10.0f, bounds.getBottom());

			// Beschriftung
			juce::String label;

			if (freq >= 1000.0f)	label = juce::String(freq / 1000.0f, 0) + "k";
			else					label = juce::String((int)freq);

			g.drawText(label,
				(int)x - 20,
				(int)bounds.getBottom() - 20,
				40,
				15,
				juce::Justification::centred);
		}

	}

	void resized() override
	{

		auto newSize = std::max(1, getWidth());

		scopeData.resize(newSize);
		fftSmoothed.resize(newSize);
		fftLookup.resize(newSize);

		rebuildFFTLookup();


	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScopeComponent)
};