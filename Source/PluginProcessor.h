/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Diffuser.h"

const int MULTICHANNEL_TOTAL_INPUTS = 2;
const int MATRIX_SIZE = 4;
//==============================================================================
/**
*/
class Reverb_Plugin_Colon_BracketAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    Reverb_Plugin_Colon_BracketAudioProcessor();
    ~Reverb_Plugin_Colon_BracketAudioProcessor() override;

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

private:
    //==============================================================================
    juce::AudioBuffer<float> delayBuffer;
    juce::AudioBuffer<float> multibuffer;
    void fillBuffer(juce::AudioBuffer<float>& buffer, int bufferSize, int dBufferSize, int channel);
    void readFromBuffer(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& delayBuffer, int channel, int bufferSize, int dBufferSize);
    juce::dsp::Matrix<float> householderMatrix{ juce::dsp::Matrix<float>(MATRIX_SIZE, MATRIX_SIZE) };
    juce::dsp::Matrix<float> inputMatrix{ juce::dsp::Matrix<float>(MATRIX_SIZE, MATRIX_SIZE) };
    juce::dsp::Matrix<float> outputMatrix{ juce::dsp::Matrix<float>(MATRIX_SIZE, MATRIX_SIZE) };
    int write_pos{ 0 };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Reverb_Plugin_Colon_BracketAudioProcessor)

    Diffuser diffuse;
};
