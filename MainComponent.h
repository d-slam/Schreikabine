#pragma once

#include <JuceHeader.h>
#include "ScopeComponent.h"
#include "SpecComponent.h"
#include "AudioState.h"
#include "UIComponent.h"
#include "AudioGeraete.h"

//==============================================================================
/*
	This component lives inside our window, and this is where you should put all
	your controls and content.
*/
class MainComponent : public juce::AudioAppComponent
{
public:
	//==============================================================================
	MainComponent();
	~MainComponent() override;

	//==============================================================================
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
	void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
	void releaseResources() override;

	//==============================================================================
	void paint(juce::Graphics& g) override;
	void resized() override;

private:

	AudioState audioState;

	std::unique_ptr<UIComponent> uiComponent;
	std::unique_ptr<ScopeComponent> scopeComponent;
	std::unique_ptr<SpecComponent> specComponent;
	std::unique_ptr<AudioGeraete> audioGeraet;


	juce::dsp::ProcessorDuplicator<
		juce::dsp::IIR::Filter<float>,
		juce::dsp::IIR::Coefficients<float>
	> filter;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
