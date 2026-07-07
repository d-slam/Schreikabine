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

		sld_hp.reset(new juce::Slider("slider_hp"));
		addAndMakeVisible(sld_hp.get());
		sld_hp->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
		sld_hp->setRange(30, 5000 , 1);
		sld_hp->setValue(audioState.hp_cutoff.load());
		sld_hp->setSkewFactorFromMidPoint(100);
		sld_hp->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, elementWidth, labelHeight);
		sld_hp->onValueChange = [this] {audioState.hp_cutoff.store(static_cast<double>(sld_hp->getValue()));};
		sld_hp->setBounds(elementWidth * 0, (labelHeight * 4) + (elementHeight * 4), elementWidth, elementHeight);
		sld_hp->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::lime);

		lbl_hp.reset(new juce::Label("label_hp"));
		addAndMakeVisible(lbl_hp.get());
		lbl_hp->setText("hp cutoff", juce::NotificationType::dontSendNotification);
		lbl_hp->setColour(juce::Label::outlineColourId, juce::Colours::lime);
		lbl_hp->setJustificationType(juce::Justification::centred);
		lbl_hp->attachToComponent(sld_hp.get(), false);

		///////////////////////////////////////////
		// particle parameters (ScopeComponent)

		sld_particleGravity.reset(new juce::Slider("slider_particleGravity"));
		addAndMakeVisible(sld_particleGravity.get());
		sld_particleGravity->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
		sld_particleGravity->setRange(0.0, 1000.0, 1.0);
		sld_particleGravity->setValue((double)audioState.particleGravity.load());
		sld_particleGravity->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, elementWidth, labelHeight);
		sld_particleGravity->onValueChange = [this] { audioState.particleGravity.store(static_cast<float>(sld_particleGravity->getValue())); };
		sld_particleGravity->setBounds(elementWidth * 3, labelHeight, elementWidth, elementHeight);
		sld_particleGravity->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::lime);

		lbl_particleGravity.reset(new juce::Label("label_particleGravity"));
		addAndMakeVisible(lbl_particleGravity.get());
		lbl_particleGravity->setText("particle g", juce::NotificationType::dontSendNotification);
		lbl_particleGravity->setColour(juce::Label::outlineColourId, juce::Colours::lime);
		lbl_particleGravity->setJustificationType(juce::Justification::centred);
		lbl_particleGravity->attachToComponent(sld_particleGravity.get(), false);

		sld_particleInitVy.reset(new juce::Slider("slider_particleInitVy"));
		addAndMakeVisible(sld_particleInitVy.get());
		sld_particleInitVy->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
		sld_particleInitVy->setRange(0.0, 400.0, 1.0);
		sld_particleInitVy->setValue((double)audioState.particleInitVy.load());
		sld_particleInitVy->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, elementWidth, labelHeight);
		sld_particleInitVy->onValueChange = [this] { audioState.particleInitVy.store(static_cast<float>(sld_particleInitVy->getValue())); };
		sld_particleInitVy->setBounds(elementWidth * 4, labelHeight, elementWidth, elementHeight);
		sld_particleInitVy->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::lime);

		lbl_particleInitVy.reset(new juce::Label("label_particleInitVy"));
		addAndMakeVisible(lbl_particleInitVy.get());
		lbl_particleInitVy->setText("init vy", juce::NotificationType::dontSendNotification);
		lbl_particleInitVy->setColour(juce::Label::outlineColourId, juce::Colours::lime);
		lbl_particleInitVy->setJustificationType(juce::Justification::centred);
		lbl_particleInitVy->attachToComponent(sld_particleInitVy.get(), false);

		sld_particleFadeRate.reset(new juce::Slider("slider_particleFadeRate"));
		addAndMakeVisible(sld_particleFadeRate.get());
		sld_particleFadeRate->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
		sld_particleFadeRate->setRange(0.0, 5.0, 0.01);
		sld_particleFadeRate->setValue((double)audioState.particleFadeRate.load());
		sld_particleFadeRate->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, elementWidth, labelHeight);
		sld_particleFadeRate->onValueChange = [this] { audioState.particleFadeRate.store(static_cast<float>(sld_particleFadeRate->getValue())); };
		sld_particleFadeRate->setBounds(elementWidth * 3, (labelHeight * 2) + (elementHeight * 2), elementWidth, elementHeight);
		sld_particleFadeRate->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::lime);

		lbl_particleFadeRate.reset(new juce::Label("label_particleFadeRate"));
		addAndMakeVisible(lbl_particleFadeRate.get());
		lbl_particleFadeRate->setText("fade/s", juce::NotificationType::dontSendNotification);
		lbl_particleFadeRate->setColour(juce::Label::outlineColourId, juce::Colours::lime);
		lbl_particleFadeRate->setJustificationType(juce::Justification::centred);
		lbl_particleFadeRate->attachToComponent(sld_particleFadeRate.get(), false);

		sld_particleRadius.reset(new juce::Slider("slider_particleRadius"));
		addAndMakeVisible(sld_particleRadius.get());
		sld_particleRadius->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
		sld_particleRadius->setRange(0.1, 8.0, 0.1);
		sld_particleRadius->setValue((double)audioState.particleRadius.load());
		sld_particleRadius->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, elementWidth, labelHeight);
		sld_particleRadius->onValueChange = [this] { audioState.particleRadius.store(static_cast<float>(sld_particleRadius->getValue())); };
		sld_particleRadius->setBounds(elementWidth * 4, (labelHeight * 2) + (elementHeight * 2), elementWidth, elementHeight);
		sld_particleRadius->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::lime);

		lbl_particleRadius.reset(new juce::Label("label_particleRadius"));
		addAndMakeVisible(lbl_particleRadius.get());
		lbl_particleRadius->setText("radius", juce::NotificationType::dontSendNotification);
		lbl_particleRadius->setColour(juce::Label::outlineColourId, juce::Colours::lime);
		lbl_particleRadius->setJustificationType(juce::Justification::centred);
		lbl_particleRadius->attachToComponent(sld_particleRadius.get(), false);

		sld_particleSpawnStep.reset(new juce::Slider("slider_particleSpawnStep"));
		addAndMakeVisible(sld_particleSpawnStep.get());
		sld_particleSpawnStep->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
		sld_particleSpawnStep->setRange(1, 64, 1);
		sld_particleSpawnStep->setValue((int)audioState.particleSpawnStep.load());
		sld_particleSpawnStep->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, elementWidth, labelHeight);
		sld_particleSpawnStep->onValueChange = [this] { audioState.particleSpawnStep.store(static_cast<int>(sld_particleSpawnStep->getValue())); };
		sld_particleSpawnStep->setBounds(elementWidth * 5, labelHeight, elementWidth, elementHeight);
		sld_particleSpawnStep->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::lime);

		lbl_particleSpawnStep.reset(new juce::Label("label_particleSpawnStep"));
		addAndMakeVisible(lbl_particleSpawnStep.get());
		lbl_particleSpawnStep->setText("spawn step", juce::NotificationType::dontSendNotification);
		lbl_particleSpawnStep->setColour(juce::Label::outlineColourId, juce::Colours::lime);
		lbl_particleSpawnStep->setJustificationType(juce::Justification::centred);
		lbl_particleSpawnStep->attachToComponent(sld_particleSpawnStep.get(), false);

		sld_particleMaxCount.reset(new juce::Slider("slider_particleMaxCount"));
		addAndMakeVisible(sld_particleMaxCount.get());
		sld_particleMaxCount->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
		sld_particleMaxCount->setRange(100, 20000, 100);
		sld_particleMaxCount->setValue((int)audioState.particleMaxCount.load());
		sld_particleMaxCount->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, elementWidth, labelHeight);
		sld_particleMaxCount->onValueChange = [this] { audioState.particleMaxCount.store(static_cast<int>(sld_particleMaxCount->getValue())); };
		sld_particleMaxCount->setBounds(elementWidth * 5, (labelHeight * 2) + (elementHeight * 2), elementWidth, elementHeight);
		sld_particleMaxCount->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::lime);

		lbl_particleMaxCount.reset(new juce::Label("label_particleMaxCount"));
		addAndMakeVisible(lbl_particleMaxCount.get());
		lbl_particleMaxCount->setText("max cnt", juce::NotificationType::dontSendNotification);
		lbl_particleMaxCount->setColour(juce::Label::outlineColourId, juce::Colours::lime);
		lbl_particleMaxCount->setJustificationType(juce::Justification::centred);
		lbl_particleMaxCount->attachToComponent(sld_particleMaxCount.get(), false);

	}

	~UIComponent()
	{
		sld_dbMin,			lbl_dbMin = nullptr;
		sld_FFTSmooth,		lbl_FFTSmooth = nullptr;
		sld_DisplaySmooth,	lbl_DisplaySmooth = nullptr;
		sld_dBVisible,		lbl_dBVisible = nullptr;
		sld_dBFloor,		lbl_dBFloor = nullptr;
		sld_hp,				lbl_hp = nullptr;

		sld_particleGravity, lbl_particleGravity = nullptr;
		sld_particleInitVy, lbl_particleInitVy = nullptr;
		sld_particleFadeRate, lbl_particleFadeRate = nullptr;
		sld_particleRadius, lbl_particleRadius = nullptr;
		sld_particleSpawnStep, lbl_particleSpawnStep = nullptr;
		sld_particleMaxCount, lbl_particleMaxCount = nullptr;
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

	std::unique_ptr<juce::Slider> sld_hp;
	std::unique_ptr<juce::Label> lbl_hp;

	// particle controls
	std::unique_ptr<juce::Slider> sld_particleGravity;
	std::unique_ptr<juce::Label> lbl_particleGravity;

	std::unique_ptr<juce::Slider> sld_particleInitVy;
	std::unique_ptr<juce::Label> lbl_particleInitVy;

	std::unique_ptr<juce::Slider> sld_particleFadeRate;
	std::unique_ptr<juce::Label> lbl_particleFadeRate;

	std::unique_ptr<juce::Slider> sld_particleRadius;
	std::unique_ptr<juce::Label> lbl_particleRadius;

	std::unique_ptr<juce::Slider> sld_particleSpawnStep;
	std::unique_ptr<juce::Label> lbl_particleSpawnStep;

	std::unique_ptr<juce::Slider> sld_particleMaxCount;
	std::unique_ptr<juce::Label> lbl_particleMaxCount;


private:

	int labelHeight = 20;

	int elementWidth = 60;
	int elementHeight = 60;

	AudioState& audioState;
};
