/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Reverb_Plugin_Colon_BracketAudioProcessor::Reverb_Plugin_Colon_BracketAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

Reverb_Plugin_Colon_BracketAudioProcessor::~Reverb_Plugin_Colon_BracketAudioProcessor()
{
}

//==============================================================================
const juce::String Reverb_Plugin_Colon_BracketAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Reverb_Plugin_Colon_BracketAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Reverb_Plugin_Colon_BracketAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Reverb_Plugin_Colon_BracketAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Reverb_Plugin_Colon_BracketAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Reverb_Plugin_Colon_BracketAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Reverb_Plugin_Colon_BracketAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Reverb_Plugin_Colon_BracketAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Reverb_Plugin_Colon_BracketAudioProcessor::getProgramName (int index)
{
    return {};
}

void Reverb_Plugin_Colon_BracketAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Reverb_Plugin_Colon_BracketAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{


    auto dBuffSize = sampleRate * 2.0f;
    delayBuffer.setSize(MULTICHANNEL_TOTAL_INPUTS, static_cast<int>(dBuffSize));
    delayBuffer.clear();
    multibuffer.setSize(MULTICHANNEL_TOTAL_INPUTS, samplesPerBlock);
    multibuffer.clear();
    outputMatrix.clear();


    // setting householder matrix
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            if (i == j) {
                householderMatrix(i, j) = -.5;
            }
            else {
                householderMatrix(i, j) = .5;
            }
        }
    }

    diffuse.prepare(sampleRate, MULTICHANNEL_TOTAL_INPUTS, samplesPerBlock);
}

void Reverb_Plugin_Colon_BracketAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Reverb_Plugin_Colon_BracketAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
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

void Reverb_Plugin_Colon_BracketAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();


    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());


    auto bufferSize = buffer.getNumSamples();
    auto dBufferSize = delayBuffer.getNumSamples();
    auto mBufferSize = multibuffer.getNumSamples();

    multibuffer.clear();
    for (int channels = 0; channels < MULTICHANNEL_TOTAL_INPUTS; channels++) {
        int original_channel = channels % getTotalNumInputChannels();
        multibuffer.copyFromWithRamp(channels, 0, buffer.getReadPointer(original_channel), bufferSize, .5f, .5f);
    }

    for (int channel = 0; channel < MULTICHANNEL_TOTAL_INPUTS; ++channel)
    {
        diffuse.diffuseBuffer(multibuffer);
        /*fillBuffer(multibuffer, bufferSize, dBufferSize, channel);
        readFromBuffer(buffer, delayBuffer, channel, bufferSize, dBufferSize);
        fillBuffer(multibuffer, bufferSize, dBufferSize, channel);*/
        int original_channel = channel % getTotalNumInputChannels();
        buffer.copyFromWithRamp(original_channel, 0, multibuffer.getReadPointer(channel), bufferSize, .5f, .5f);
    }
    /*write_pos += bufferSize;
    write_pos %= dBufferSize;*/

    //buffer.applyGain(2);
}
void Reverb_Plugin_Colon_BracketAudioProcessor::readFromBuffer(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& dBuffer, int channel, int bufferSize, int dBufferSize) {

    auto readPosition = write_pos - (5000 + (3000*channel+1));

    const float gainDivisor = 4.f;

    if (readPosition < 0)
        readPosition += dBufferSize;

   auto* dataArr = dBuffer.getArrayOfWritePointers();
   for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            inputMatrix(i, j) = dataArr[j][readPosition];
        }
    }

    outputMatrix = inputMatrix * householderMatrix;

    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            dataArr[j][readPosition] = outputMatrix(i, j);
        }
    }
    int original_channel = channel % getNumInputChannels();
    if (readPosition + bufferSize < dBufferSize)
    {
        buffer.addFromWithRamp(original_channel, 0, dBuffer.getReadPointer(channel, readPosition), bufferSize, gainDivisor, gainDivisor);
    }

    else {
        auto numSamplesToEnd = dBufferSize - readPosition;
        buffer.addFromWithRamp(original_channel, 0, dBuffer.getReadPointer(channel, readPosition), numSamplesToEnd, gainDivisor, gainDivisor);

        auto numSamplesAtStart = bufferSize - numSamplesToEnd;
        buffer.addFromWithRamp(original_channel, numSamplesToEnd, delayBuffer.getReadPointer(channel, 0), numSamplesAtStart, gainDivisor, gainDivisor);
    }

    for (int channels = 0; channels < MULTICHANNEL_TOTAL_INPUTS; channels++) {
        int original_channel = channels % getTotalNumInputChannels();
        multibuffer.copyFromWithRamp(channels, 0, buffer.getReadPointer(original_channel), bufferSize, .5f, .5f);
    }

}
void Reverb_Plugin_Colon_BracketAudioProcessor::fillBuffer(juce::AudioBuffer<float>& buffer, int bufferSize, int dBufferSize, int channel){
    


        auto* channelData = multibuffer.getWritePointer(channel);
        if (dBufferSize > bufferSize + write_pos)
        {
            delayBuffer.copyFromWithRamp(channel, write_pos, channelData, bufferSize, .1f,  .1f);
        }
        else
        {
            auto numSamplesToEnd = dBufferSize - write_pos;
            delayBuffer.copyFromWithRamp(channel, write_pos, channelData, numSamplesToEnd,  .1f,  .1f);

            auto numSamplesAtStart = bufferSize - numSamplesToEnd;
            delayBuffer.copyFromWithRamp(channel, 0, channelData + numSamplesToEnd, numSamplesAtStart,  .1f,  .1f);
        }

    
}

//==============================================================================
bool Reverb_Plugin_Colon_BracketAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Reverb_Plugin_Colon_BracketAudioProcessor::createEditor()
{
    //return new Reverb_Plugin_Colon_BracketAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void Reverb_Plugin_Colon_BracketAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Reverb_Plugin_Colon_BracketAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Reverb_Plugin_Colon_BracketAudioProcessor();
}
