/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Params.h"

//==============================================================================
/**
*/
class SelfModulatorAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SelfModulatorAudioProcessor();
    ~SelfModulatorAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    void calcFilterCoefficients();

private:
    inline float ModulationCurve(float x) const { return pow(2.0, DepthCurve * (x - 1)) - CurveOffset; }

    inline float getParamValue(param::PID pid) {
        const int iPid = static_cast<int>(pid);
        const auto normVal = params[iPid]->getValue();
        return params[iPid]->getNormalisableRange().convertFrom0to1(normVal);
    }

public:
    using RAPPtr = juce::RangedAudioParameter*;
    using RAPPtrArray = std::array<RAPPtr, param::NumParams>;

    juce::AudioProcessorValueTreeState apvts;
    RAPPtrArray params;

private:
    double lastSampleRate;
    double lastQ;
    int lastBlockSize;

    juce::dsp::DelayLine<float> variableDelay;

    float lastCutoffFreq;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> lpf;

    const double MaxDepth = 0.1; // in seconds
    const double DepthCurve = 6.0;
    const double CurveOffset = pow(2.0, -DepthCurve);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SelfModulatorAudioProcessor)
};
