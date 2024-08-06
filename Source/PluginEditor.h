/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class Reverb_Plugin_Colon_BracketAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Reverb_Plugin_Colon_BracketAudioProcessorEditor (Reverb_Plugin_Colon_BracketAudioProcessor&);
    ~Reverb_Plugin_Colon_BracketAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Reverb_Plugin_Colon_BracketAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Reverb_Plugin_Colon_BracketAudioProcessorEditor)
};
