/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BasicChorusAudioProcessor::BasicChorusAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts (*this, nullptr, "Parameters", createParameters())
#endif
{
    apvts.addParameterListener ("RATE", this);
    apvts.addParameterListener ("DEPTH", this);
    apvts.addParameterListener ("CENTREDELAY", this);
    apvts.addParameterListener ("FEEDBACK", this);
    apvts.addParameterListener ("MIX", this);
}

BasicChorusAudioProcessor::~BasicChorusAudioProcessor()
{
    apvts.removeParameterListener ("RATE", this);
    apvts.removeParameterListener ("DEPTH", this);
    apvts.removeParameterListener ("CENTREDELAY", this);
    apvts.removeParameterListener ("FEEDBACK", this);
    apvts.removeParameterListener ("MIX", this);
}

//==============================================================================
const String BasicChorusAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BasicChorusAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BasicChorusAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BasicChorusAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BasicChorusAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BasicChorusAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BasicChorusAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BasicChorusAudioProcessor::setCurrentProgram (int index)
{
}

const String BasicChorusAudioProcessor::getProgramName (int index)
{
    return {};
}

void BasicChorusAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void BasicChorusAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    //spec.numChannels = getTotalNumInputChannels();
    
    chorus.prepare (spec);
    chorus.reset();
    isInit.store (true);
}

void BasicChorusAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BasicChorusAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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

void BasicChorusAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    if (! isInit.load())
        return;
    
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    dsp::AudioBlock<float> sampleBlock (buffer);
    
    chorus.process (dsp::ProcessContextReplacing<float> (sampleBlock));
}

//==============================================================================
bool BasicChorusAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* BasicChorusAudioProcessor::createEditor()
{
    return new BasicChorusAudioProcessorEditor (*this);
}

//==============================================================================
void BasicChorusAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    ValueTree copyState = apvts.copyState();
    std::unique_ptr<XmlElement> xml = copyState.createXml();
    copyXmlToBinary (*xml.get(), destData);
}

void BasicChorusAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xml = getXmlFromBinary (data, sizeInBytes);
    ValueTree copyState = ValueTree::fromXml (*xml.get());
    apvts.replaceState (copyState);
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BasicChorusAudioProcessor();
}

void BasicChorusAudioProcessor::reset()
{
    chorus.reset();
}

void BasicChorusAudioProcessor::parameterChanged (const String& parameterID, float newValue)
{
    if (parameterID == "RATE")
        chorus.setRate (newValue);
    
    if (parameterID == "DEPTH")
        chorus.setDepth (newValue);
    
    if (parameterID == "CENTREDELAY")
        chorus.setCentreDelay (newValue);
    
    if (parameterID == "FEEDBACK")
        chorus.setFeedback (newValue);
    
    if (parameterID == "MIX")
        chorus.setMix (newValue);
}

AudioProcessorValueTreeState::ParameterLayout BasicChorusAudioProcessor::createParameters()
{
    AudioProcessorValueTreeState::ParameterLayout params;
    
    using Range = NormalisableRange<float>;
    
    params.add (std::make_unique<AudioParameterInt>  ("RATE", "Rate", 0, 99, 0));
    params.add (std::make_unique<AudioParameterFloat>("DEPTH", "Depth", Range { 0.0f, 1.0f, 0.01f }, 0.0f));
    params.add (std::make_unique<AudioParameterInt>  ("CENTREDELAY", "Centre Delay", 1, 100, 1));
    params.add (std::make_unique<AudioParameterFloat>("FEEDBACK", "Feedback", Range { -1.0f, 1.0f, 0.01f }, 0.0f));
    params.add (std::make_unique<AudioParameterFloat>("MIX", "Mix", Range { 0.0f, 1.0f, 0.01f }, 0.0f));
    
    return params;
}
