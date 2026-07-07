#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
	setAudioChannels(2, 0);  // we want a couple of input channels but no outputs

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

	// lesen
	if (bufferToFill.buffer->getNumChannels() > 0)
	{
		auto* channelData = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);

		for (auto i = 0; i < bufferToFill.numSamples; ++i)
		{
			scopeComponent->pushNextSampleIntoFifo(channelData[i]);
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
	juce::Rectangle<int> boundsUi(0, 0, getWidth() / 2, getHeight() / 2);
	juce::Rectangle<int> boundsScope(0, 0, getWidth() / 1, getHeight() / 1);
	juce::Rectangle<int> boundsGeraet(getWidth() / 2, getHeight() / 2, getWidth() / 2, getHeight() / 2);

	scopeComponent->setBounds(boundsScope);
	uiComponent->setBounds(boundsUi);
	audioGeraet->setBounds(boundsGeraet);
}
