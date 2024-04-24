/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

//==============================================================================
SelfModulatorAudioProcessorEditor::SelfModulatorAudioProcessorEditor(SelfModulatorAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , audioProcessor(p)
    , sliders{
        AttachedSlider(p, param::PID::ModulationDepth),
        AttachedSlider(p, param::PID::FilterCutoff),
        AttachedSlider(p, param::PID::FilterResonance)
    } {
    setSize (300, 200);
    setResizable(true, true);

    const float maxSliderAngle = MathConstants<float>::pi * 0.75;
    const float startSliderAngle = -maxSliderAngle + MathConstants<float>::twoPi;
    const float endSliderAngle = maxSliderAngle + MathConstants<float>::twoPi;

    for (auto i = 0; i < sliders.size(); ++i) {
        auto& slider = sliders[i];
        addAndMakeVisible(slider.slider);
    }
}

SelfModulatorAudioProcessorEditor::~SelfModulatorAudioProcessorEditor()
{
}

//==============================================================================
void SelfModulatorAudioProcessorEditor::paint (Graphics& g) {
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void SelfModulatorAudioProcessorEditor::resized() {
    auto w = static_cast<float>(getWidth());
    auto h = static_cast<float>(getHeight()); 
   
    const auto relativeSliderPadding = 0.1f;
    if (w >= h) {

        auto x = 0.0f;
        auto y = h * relativeSliderPadding;

        auto sliderWidth = w / static_cast<float>(sliders.size());
        auto sliderHeight = h * (1 - relativeSliderPadding * 2.0f);

        for (auto i = 0; i < sliders.size(); ++i) {
            sliders[i].slider.setBounds(juce::Rectangle<float>(x, y, sliderWidth, sliderHeight).toNearestInt());
            x += sliderWidth;
        }
    } else {
        auto x = w * relativeSliderPadding;
        auto y = 0.0f;

        auto sliderWidth = w * (1 - relativeSliderPadding * 2.0f);
        auto sliderHeight = h / static_cast<float>(sliders.size());
        for (auto i = 0; i < sliders.size(); ++i) {
            sliders[i].slider.setBounds(juce::Rectangle<float>(x, y, sliderWidth , sliderHeight).toNearestInt());
            y += sliderHeight;
        }
    }
}
