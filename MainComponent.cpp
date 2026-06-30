#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
	setAudioChannels(2, 0);  // we want a couple of input channels but no outputs

	uiComponent.reset(new UIComponent(audioState));			addAndMakeVisible(uiComponent.get());
	scopeComponent.reset(new ScopeComponent(audioState));	addAndMakeVisible(scopeComponent.get());
	specComponent.reset(new SpecComponent(audioState));		addAndMakeVisible(specComponent.get());
	audioGeraet.reset(new AudioGeraete());					addAndMakeVisible(audioGeraet.get());


	setSize(1280, 1024);
}

MainComponent::~MainComponent()
{
	shutdownAudio();
	uiComponent, scopeComponent, specComponent, audioGeraet = nullptr;
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

}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
	
	*filter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, audioState.lp_freq.load()); // 1 kHz Tiefpass
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
			specComponent->pushNextSampleIntoFifo(channelData[i]);
		}

	}

}

void MainComponent::releaseResources()
{

}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::black);

}

void MainComponent::resized()
{

	juce::Rectangle<int> boundsUi(0, 0, getWidth() / 2, getHeight() / 2);
	juce::Rectangle<int> boundsScope(getWidth() / 2, 0, getWidth() / 2, getHeight() / 2);
	juce::Rectangle<int> boundsSpec(0, getHeight() / 2, getWidth() / 2, getHeight() / 2);
	juce::Rectangle<int> boundsGeraet(getWidth() / 2, getHeight() / 2, getWidth() / 2, getHeight() / 2);

	uiComponent->setBounds(boundsUi);
	scopeComponent->setBounds(boundsScope);
	specComponent->setBounds(boundsSpec);
	audioGeraet->setBounds(boundsGeraet);
}
