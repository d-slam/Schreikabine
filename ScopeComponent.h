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

// OpenGL support removed for minimal non-GL build

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
		setBufferedToImage(true); // use component buffering

		// higher update rate for smoother rendering
		startTimerHz(120);

		// create offscreen image for double-buffered drawing
		if (getWidth() > 0 && getHeight() > 0)
			spectrumImage = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);

		// enable OpenGL acceleration for this component (if module available)
		// OpenGL disabled in this build - use CPU painting only
	}

	~ScopeComponent() {}

	// wichter optimisierungscheis!!! mit steigender fftOrder brauchen wir lange bis fifo voll ist, deswegen öffters rendern mit /8 für 14te zB
	void pushNextSampleIntoFifo(float sample) noexcept
	{
		if (fifoIndex == fftSize / 4)
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
		float nyquist = audioState.currentSampleRate.load() * 0.5f;

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

	static constexpr int fftOrder = 13;		//def 11
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

	// Offscreen image and path for double-buffered drawing
	juce::Image spectrumImage;
	juce::Path spectrumPath;
	// protects concurrent access to spectrumImage between GUI thread and GL render thread
	juce::CriticalSection spectrumImageLock;


	// OpenGL removed - using CPU rendering only
	// OpenGL support disabled - CPU-only rendering


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

		forwardFFT.performFrequencyOnlyForwardTransform(fftData);

		auto mindB = (float)audioState.dbMin.load();
		auto maxdB = 0.0f;

		// compute magnitudes and apply smoothing
		for (size_t i = 0; i < scopeData.size(); ++i)
		{
			const auto& l = fftLookup[i];

			float fftValue = fftData[l.bin0] + l.interp * (fftData[l.bin1] - fftData[l.bin0]);

			fftSmoothed[i] = fftSmoothed[i] * audioState.fftSmooth.load() + fftValue * (1.0f - audioState.fftSmooth.load());

			float db = juce::Decibels::gainToDecibels(fftSmoothed[i] + 1e-6f) - juce::Decibels::gainToDecibels((float)fftSize);

			float level = juce::jmap(juce::jlimit(mindB, maxdB, db), mindB, maxdB, 0.0f, 1.0f);

			scopeData[i] = scopeData[i] + audioState.displaySmooth.load() * (level - scopeData[i]);
		}

		// draw into offscreen image to reduce paint() workload
		{
			juce::ScopedLock lock(spectrumImageLock);
			if (spectrumImage.isNull() || spectrumImage.getWidth() != getWidth() || spectrumImage.getHeight() != getHeight())
			{
				if (getWidth() > 0 && getHeight() > 0)
					spectrumImage = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
			}

			if (!spectrumImage.isNull())
			{
				juce::Graphics gi(spectrumImage);
				gi.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
				gi.fillAll(juce::Colours::black);

				spectrumPath.clear();
				spectrumPath.startNewSubPath(0.0f, (float)getHeight());

				for (size_t i = 1; i < scopeData.size(); ++i)
				{
					auto x = juce::jmap<float>(
						static_cast<float>(i),
						0.0f,
						static_cast<float>(scopeData.size() - 1),
						0.0f,
						(float)getWidth());

					auto y = juce::jmap<float>(
						scopeData[i],
						0.0f,
						1.0f,
						(float)getHeight(),
						0.0f);

					spectrumPath.lineTo(x, y);
				}

				gi.setColour(juce::Colours::lime.withAlpha(0.2f));
				gi.fillPath(spectrumPath);

				// subtle bloom/glow: smaller, lower-alpha layered strokes for a gentler look
				gi.setColour(juce::Colours::lime.withAlpha(0.125f));
				gi.strokePath(spectrumPath, juce::PathStrokeType(12.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
				gi.setColour(juce::Colours::lime.withAlpha(0.112f));
				gi.strokePath(spectrumPath, juce::PathStrokeType(28.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

				// main stroke
				gi.setColour(juce::Colours::lime.withAlpha(0.95f));
				gi.strokePath(spectrumPath, juce::PathStrokeType(1.5f));
			}
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

		auto bounds = getLocalBounds().toFloat();

		// If we have an offscreen rendered image, blit it — much cheaper than re-drawing the path every frame
		if (!spectrumImage.isNull())
		{
			g.drawImageAt(spectrumImage, 0, 0);
		}
		else
		{
			g.fillAll(juce::Colours::black);
			g.setColour(juce::Colours::lime);

			auto bounds = getLocalBounds().toFloat();

			juce::Path tmpPath;
			tmpPath.startNewSubPath(0.0f, bounds.getBottom());

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

				tmpPath.lineTo(x, y);
			}

			g.setColour(juce::Colours::lime.withAlpha(0.2f));
			g.fillPath(tmpPath);

			g.setColour(juce::Colours::lime.withAlpha(0.9f));
			g.strokePath(tmpPath, juce::PathStrokeType(1.5f));
		}


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
			g.drawVerticalLine((int)x, (int)bounds.getBottom() - 10, (int)bounds.getBottom());

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