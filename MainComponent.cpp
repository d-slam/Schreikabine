#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
	setAudioChannels(2, 0);  // we want a couple of input channels but no outputs
	setWantsKeyboardFocus(true);
	setMouseClickGrabsKeyboardFocus(true);

	scopeComponent.reset(new ScopeComponent(audioState));	addAndMakeVisible(scopeComponent.get());
	uiComponent.reset(new UIComponent(audioState));			addAndMakeVisible(uiComponent.get());
	audioGeraet.reset(new AudioGeraete());					/*addAndMakeVisible(audioGeraet.get());*/

	setSize(1280, 1024);
}

MainComponent::~MainComponent()
{
	shutdownAudio();
	uiComponent, scopeComponent, audioGeraet = nullptr;
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{

	audioState.currentSampleRate.store(sampleRate);

	/// sollong wir nicht in runtime die sampleRate ändern sollte de scheise halten...
	//lastSampleRate = sampleRate;

	//if (lastSampleRate != audioState.currentSampleRate.load())
	//{
	//	lastSampleRate = audioState.currentSampleRate.load();
	//	scopeComponent->rebuildFFTLookup();
	//}

	currentSampleRate = sampleRate;

	//filter init
	juce::dsp::ProcessSpec spec;
	spec.sampleRate = sampleRate;
	spec.maximumBlockSize = samplesPerBlockExpected;
	spec.numChannels = 2;

	filter.prepare(spec);

	// initialize 4th-order high-pass coefficients (two cascaded 2nd-order stages)
	float cutoff = (float)audioState.hp_cutoff.load();
	lastLpFreq = cutoff;
	auto coeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, cutoff);
	*filter.get<0>().state = *coeffs;
	*filter.get<1>().state = *coeffs;

}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
	// update coefficients if cutoff changed
	float cutoff = (float)audioState.hp_cutoff.load();
	if (std::abs(cutoff - lastLpFreq) > 0.001f)
	{
		auto coeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, cutoff);
		*filter.get<0>().state = *coeffs;
		*filter.get<1>().state = *coeffs;
		lastLpFreq = cutoff;
	}
	//filtern
	auto& buffer = *bufferToFill.buffer;

	juce::dsp::AudioBlock<float> block(
		buffer.getArrayOfWritePointers(),
		buffer.getNumChannels(),
		bufferToFill.startSample,
		bufferToFill.numSamples);

	juce::dsp::ProcessContextReplacing<float> context(block);

	filter.process(context);

	// lesen + apply gain from UI
	if (bufferToFill.buffer->getNumChannels() > 0)
	{
		auto* writePtr = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
		float gain = (float)juce::Decibels::decibelsToGain((double)audioState.gain_dB.load());

		for (auto i = 0; i < bufferToFill.numSamples; ++i)
		{
			float s = writePtr[i] * gain;
			writePtr[i] = s;
			scopeComponent->pushNextSampleIntoFifo(s);
		}

	}

}

void MainComponent::releaseResources()
{
	//filter.reset();
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::black);
}

void MainComponent::resized()
{
	// Layout: Scope on the left, UI on the right column, audio device panel below UI
	auto area = getLocalBounds();

	// reserve right column for UI (approx 1/3 of width, min 220px)
	//int rightWidth = juce::jmax(220, area.getWidth() / 3);
	//auto rightCol = area.removeFromRight(rightWidth);

	// scope takes remaining (left) area
	scopeComponent->setBounds(area);

	// UI gets top portion of right column
	//int uiHeight = rightCol.getHeight() * 2 / 3;
	//auto uiBounds = rightCol.removeFromTop(uiHeight);
	if (uiVisible)
		uiComponent->setBounds(area);

	// audio device panel (if present) takes the remaining bottom part of right column
	//audioGeraet->setBounds(rightCol);
}

bool MainComponent::keyPressed(const juce::KeyPress& key)
{
	if (key.getTextCharacter() == 'm' || key.getTextCharacter() == 'M')
	{
		uiVisible = !uiVisible;
		if (uiComponent != nullptr)
			uiComponent->setVisible(uiVisible);
		resized();
		repaint();
		return true;
	}

	return false;
}

void MainComponent::visibilityChanged()
{
	if (isShowing())
		grabKeyboardFocus();
}
