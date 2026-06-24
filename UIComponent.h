/*
  ==============================================================================

	UIComponent.h
	Created: 19 Jun 2026 10:20:31am
	Author:  SchoeDam

  ==============================================================================
*/

#pragma once
#include "AudioState.h"


class UIComponent : public juce::Component
{
public:
	UIComponent(AudioState& state) :audioState(state)
	{
		////// ScopeComponent
		///////////////////////////////////////////	dbMin

		sld_dbMin.reset(new juce::Slider("slider_dbMin"));
		addAndMakeVisible(sld_dbMin.get());
		sld_dbMin->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
		sld_dbMin->setRange(-100, 0, 1);
		sld_dbMin->setTextValueSuffix(" dB");
		sld_dbMin->setValue(-100);
		sld_dbMin->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, elementWidth, labelHeight);
		sld_dbMin->onValueChange = [this] {audioState.dbMin.store(static_cast<int>(sld_dbMin->getValue()));};
		sld_dbMin->setBounds(0, labelHeight, elementWidth, elementHeight);
		sld_dbMin->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::lime);

		lbl_dbMin.reset(new juce::Label("label_dbMin"));
		addAndMakeVisible(lbl_dbMin.get());
		lbl_dbMin->setText("dbMin", juce::NotificationType::dontSendNotification);
		lbl_dbMin->setColour(juce::Label::outlineColourId, juce::Colours::lime);
		lbl_dbMin->setJustificationType(juce::Justification::centred);
		lbl_dbMin->attachToComponent(sld_dbMin.get(), false);

		///////////////////////////////////////////	fftSmooth

		sld_FFTSmooth.reset(new juce::Slider("slider_FFTSmooth"));
		addAndMakeVisible(sld_FFTSmooth.get());
		sld_FFTSmooth->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
		sld_FFTSmooth->setRange(0.0f, 0.99f, 0.01f);
		sld_FFTSmooth->setValue(0.8f);
		sld_FFTSmooth->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, elementWidth, labelHeight);
		sld_FFTSmooth->onValueChange = [this] {audioState.fftSmooth.store(static_cast<float>(sld_FFTSmooth->getValue()));};
		sld_FFTSmooth->setBounds(elementWidth * 1, labelHeight, elementWidth, elementHeight);
		sld_FFTSmooth->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::lime);

		lbl_FFTSmooth.reset(new juce::Label("label_FFTSmooth"));
		addAndMakeVisible(lbl_FFTSmooth.get());
		lbl_FFTSmooth->setText("fft smooth", juce::NotificationType::dontSendNotification);
		lbl_FFTSmooth->setColour(juce::Label::outlineColourId, juce::Colours::lime);
		lbl_FFTSmooth->setJustificationType(juce::Justification::centred);
		lbl_FFTSmooth->attachToComponent(sld_FFTSmooth.get(), false);

		///////////////////////////////////////////

		sld_DisplaySmooth.reset(new juce::Slider("slider_DisplaySmooth"));
		addAndMakeVisible(sld_DisplaySmooth.get());
		sld_DisplaySmooth->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
		sld_DisplaySmooth->setRange(0.0f, 0.99f, 0.01f);
		sld_DisplaySmooth->setValue(0.15f);
		sld_DisplaySmooth->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, elementWidth, labelHeight);
		sld_DisplaySmooth->onValueChange = [this] {audioState.displaySmooth.store(static_cast<float>(sld_DisplaySmooth->getValue()));};
		sld_DisplaySmooth->setBounds(elementWidth * 2, labelHeight, elementWidth, elementHeight);
		sld_DisplaySmooth->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::lime);

		lbl_DisplaySmooth.reset(new juce::Label("label_DisplaySmooth"));
		addAndMakeVisible(lbl_DisplaySmooth.get());
		lbl_DisplaySmooth->setText("display smooth", juce::NotificationType::dontSendNotification);
		lbl_DisplaySmooth->setColour(juce::Label::outlineColourId, juce::Colours::lime);
		lbl_DisplaySmooth->setJustificationType(juce::Justification::centred);
		lbl_DisplaySmooth->attachToComponent(sld_DisplaySmooth.get(), false);

	}

	~UIComponent()
	{
		sld_dbMin, lbl_dbMin = nullptr;
		sld_FFTSmooth, lbl_FFTSmooth = nullptr;
		sld_DisplaySmooth, lbl_DisplaySmooth = nullptr;
	}

	void resize() {}

	std::unique_ptr<juce::Slider> sld_dbMin;
	std::unique_ptr<juce::Label> lbl_dbMin;


	std::unique_ptr<juce::Slider> sld_FFTSmooth;
	std::unique_ptr<juce::Label> lbl_FFTSmooth;

	std::unique_ptr<juce::Slider> sld_DisplaySmooth;
	std::unique_ptr<juce::Label> lbl_DisplaySmooth;


private:

	int labelHeight = 20;

	int elementWidth = 60;
	int elementHeight = 60;

	AudioState& audioState;
};
