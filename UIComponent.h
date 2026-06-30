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


		///////////////////////////////////////////

		sld_dBVisible.reset(new juce::Slider("slider_dBVisible"));
		addAndMakeVisible(sld_dBVisible.get());
		sld_dBVisible->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
		sld_dBVisible->setRange(-65, 0, 1);
		sld_dBVisible->setValue(-65);
		sld_dBVisible->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, elementWidth, labelHeight);
		sld_dBVisible->onValueChange = [this]
			{
				auto maxFloor = sld_dBVisible->getValue() - 1;
				sld_dBFloor->setRange(-80, maxFloor, 1);
				if (sld_dBFloor->getValue() > maxFloor)			sld_dBFloor->setValue(maxFloor, juce::dontSendNotification);
				audioState.spec_dBVisible.store(static_cast<int>(sld_dBVisible->getValue()));

			};
		sld_dBVisible->setBounds(elementWidth * 0, (labelHeight * 2) + (elementHeight * 2), elementWidth, elementHeight);
		sld_dBVisible->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::lime);

		lbl_dBVisible.reset(new juce::Label("label_dBVisible"));
		addAndMakeVisible(lbl_dBVisible.get());
		lbl_dBVisible->setText("dB Visible", juce::NotificationType::dontSendNotification);
		lbl_dBVisible->setColour(juce::Label::outlineColourId, juce::Colours::lime);
		lbl_dBVisible->setJustificationType(juce::Justification::centred);
		lbl_dBVisible->attachToComponent(sld_dBVisible.get(), false);


		///////////////////////////////////////////

		sld_dBFloor.reset(new juce::Slider("slider_dBFloor"));
		addAndMakeVisible(sld_dBFloor.get());
		sld_dBFloor->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
		sld_dBFloor->setRange(-80, sld_dBVisible->getValue() - 1, 1);
		sld_dBFloor->setValue(-80);
		sld_dBFloor->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, elementWidth, labelHeight);
		sld_dBFloor->onValueChange = [this] {audioState.spec_dBFloor.store(static_cast<int>(sld_dBFloor->getValue()));};
		sld_dBFloor->setBounds(elementWidth * 1, (labelHeight * 2) + (elementHeight * 2), elementWidth, elementHeight);
		sld_dBFloor->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::lime);

		lbl_dBFloor.reset(new juce::Label("label_dBFloor"));
		addAndMakeVisible(lbl_dBFloor.get());
		lbl_dBFloor->setText("dB Floor", juce::NotificationType::dontSendNotification);
		lbl_dBFloor->setColour(juce::Label::outlineColourId, juce::Colours::lime);
		lbl_dBFloor->setJustificationType(juce::Justification::centred);
		lbl_dBFloor->attachToComponent(sld_dBFloor.get(), false);


		///////////////////////////////////////////

		sld_lp.reset(new juce::Slider("slider_lp"));
		addAndMakeVisible(sld_lp.get());
		sld_lp->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
		sld_lp->setRange(30, 5000 , 1);
		sld_lp->setValue(30);
		sld_lp->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, elementWidth, labelHeight);
		sld_lp->onValueChange = [this] {audioState.lp_freq.store(static_cast<double>(sld_lp->getValue()));};
		sld_lp->setBounds(elementWidth * 0, (labelHeight * 4) + (elementHeight * 4), elementWidth, elementHeight);
		sld_lp->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::lime);

		lbl_lp.reset(new juce::Label("label_lp"));
		addAndMakeVisible(lbl_lp.get());
		lbl_lp->setText("lp frequ", juce::NotificationType::dontSendNotification);
		lbl_lp->setColour(juce::Label::outlineColourId, juce::Colours::lime);
		lbl_lp->setJustificationType(juce::Justification::centred);
		lbl_lp->attachToComponent(sld_lp.get(), false);

	}

	~UIComponent()
	{
		sld_dbMin,			lbl_dbMin = nullptr;
		sld_FFTSmooth,		lbl_FFTSmooth = nullptr;
		sld_DisplaySmooth,	lbl_DisplaySmooth = nullptr;
		sld_dBVisible,		lbl_dBVisible = nullptr;
		sld_dBFloor,		lbl_dBFloor = nullptr;
		sld_lp,				lbl_lp = nullptr;
	}

	void resize() {}

	//scope
	std::unique_ptr<juce::Slider> sld_dbMin;
	std::unique_ptr<juce::Label> lbl_dbMin;

	std::unique_ptr<juce::Slider> sld_FFTSmooth;
	std::unique_ptr<juce::Label> lbl_FFTSmooth;

	std::unique_ptr<juce::Slider> sld_DisplaySmooth;
	std::unique_ptr<juce::Label> lbl_DisplaySmooth;

	//spec
	std::unique_ptr<juce::Slider> sld_dBVisible;
	std::unique_ptr<juce::Label> lbl_dBVisible;

	std::unique_ptr<juce::Slider> sld_dBFloor;
	std::unique_ptr<juce::Label> lbl_dBFloor;

	std::unique_ptr<juce::Slider> sld_lp;
	std::unique_ptr<juce::Label> lbl_lp;


private:

	int labelHeight = 20;

	int elementWidth = 60;
	int elementHeight = 60;

	AudioState& audioState;
};
