/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

//==============================================================================
SelfModulatorAudioProcessor::SelfModulatorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
    , apvts(*this, nullptr, "Parameters", param::createParameterLayout())
    , params()
    , lastSampleRate{44100.0}
    , lastBlockSize{getBlockSize()} {
    for (auto i = 0; i < param::NumParams; ++i) {
        const auto pID = static_cast<param::PID>(i);
        const auto id = param::toID(pID);
        params[i] = apvts.getParameter(id);
    }

    lastCutoffFreq = getParamValue(param::PID::FilterCutoff);
    lastQ = getParamValue(param::PID::FilterResonance);
    calcFilterCoefficients();

    /*const int numChannels = getTotalNumOutputChannels();
    filters.reserve(numChannels);
    for (auto i = 0; i < numChannels; ++i) {
        filters.push_back(SinglePoleLPF(lastSampleRate));
    }*/
}

SelfModulatorAudioProcessor::~SelfModulatorAudioProcessor()
{
}

//==============================================================================
const String SelfModulatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SelfModulatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SelfModulatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SelfModulatorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SelfModulatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SelfModulatorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SelfModulatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SelfModulatorAudioProcessor::setCurrentProgram (int index)
{
}

const String SelfModulatorAudioProcessor::getProgramName (int index)
{
    return {};
}

void SelfModulatorAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void SelfModulatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {
    dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels(); 
    
    if (lastSampleRate != sampleRate) {
        lastSampleRate = sampleRate;
        lastCutoffFreq = getParamValue(param::PID::FilterCutoff);
        lastQ = getParamValue(param::PID::FilterResonance);
        calcFilterCoefficients();
    }

    if (lastBlockSize != samplesPerBlock) {
        lastBlockSize = samplesPerBlock;
    } 
    
    if (variableDelay.getMaximumDelayInSamples() != sampleRate * 3) {
        variableDelay.setMaximumDelayInSamples(sampleRate * 3);
    }

    variableDelay.prepare(spec);
    lpf.prepare(spec);
    
    /*for (auto f : filters) {
        f.prepare(spec);
    }*/
}

void SelfModulatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SelfModulatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SelfModulatorAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    const float depth = getParamValue(param::PID::ModulationDepth);
    const float bias = lastSampleRate * MaxDepth;
    const float modulationDepth = ModulationCurve(depth) * bias; // assumes signal is between -1.0 and 1.0

    dsp::AudioBlock<float> block{ buffer };
    dsp::ProcessContextReplacing<float> context{ block };

    const auto cutoffFreq = getParamValue(param::PID::FilterCutoff);
    const auto Q = getParamValue(param::PID::FilterResonance);
    if (lastCutoffFreq != cutoffFreq || lastQ != Q) {
        lastCutoffFreq = cutoffFreq;
        lastQ = Q;
        calcFilterCoefficients();
    }
    lpf.process(context);

    for (size_t channel = 0; channel < numChannels; ++channel) {
        auto* channelPtr = block.getChannelPointer(channel);
        for (size_t i = 0; i < numSamples; ++i) {
            const float newDelay = channelPtr[i] * modulationDepth + bias;
            variableDelay.setDelay(newDelay);
            variableDelay.pushSample((int)channel, channelPtr[i]);
            channelPtr[i] = variableDelay.popSample((int)channel);
        }
    }
}

//==============================================================================
bool SelfModulatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* SelfModulatorAudioProcessor::createEditor()
{
    return new SelfModulatorAudioProcessorEditor (*this);
}

//==============================================================================
void SelfModulatorAudioProcessor::getStateInformation (MemoryBlock& destData) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SelfModulatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

void SelfModulatorAudioProcessor::calcFilterCoefficients() {
    *lpf.state = *dsp::IIR::Coefficients<float>::makeLowPass(lastSampleRate, lastCutoffFreq, lastQ);
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SelfModulatorAudioProcessor();
}
