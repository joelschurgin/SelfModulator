/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "params.h"

//==============================================================================
/**
*/

// from https://github.com/Mrugalla/ParametersExample/blob/master/Source/PluginEditor.h
struct AttachedSlider
{
    using Slider = juce::Slider;
    using Attachment = juce::SliderParameterAttachment;
    using Component = juce::Component;

    AttachedSlider(SelfModulatorAudioProcessor& processor, param::PID pID) :
        slider(),
        attachment(*processor.params[static_cast<int>(pID)], slider, nullptr) {
        slider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
        slider.setTextBoxStyle(Slider::TextBoxBelow, false, slider.getTextBoxWidth(), slider.getTextBoxHeight());
    }

    Slider slider;
    Attachment attachment;
};

class SelfModulatorAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SelfModulatorAudioProcessorEditor (SelfModulatorAudioProcessor&);
    ~SelfModulatorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SelfModulatorAudioProcessor& audioProcessor;

    std::array<AttachedSlider, param::NumParams> sliders;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SelfModulatorAudioProcessorEditor)
};
