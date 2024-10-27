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
    for (auto i = 0; i < params::NumParams; ++i) {
        auto pid = static_cast<params::PID>(i);
        parameters.push_back(apvts.getParameter(params::toID(pid)));
    }


}

Reverb_Plugin_Colon_BracketAudioProcessor::~Reverb_Plugin_Colon_BracketAudioProcessor()
{
}

inline void createParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout, params::PID pid, juce::NormalisableRange<float> range, float initVal) {
    layout.add(std::make_unique<AudioParameterFloat>(params::toID(pid), params::toString(pid), range, initVal));
}
juce::AudioProcessorValueTreeState::ParameterLayout  Reverb_Plugin_Colon_BracketAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    using Range = juce::NormalisableRange<float>;
    createParameter(layout, params::PID::RoomSize, Range(20, 600), 50);
    createParameter(layout, params::PID::Decay, Range(0, 1), .8);
    createParameter(layout, params::PID::EarlyReflections, Range(0, 1), .5);
    return layout;
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

    multibuffer.setSize(MULTICHANNEL_TOTAL_INPUTS, samplesPerBlock);
    multiFeedbackBuffer.setSize(MULTICHANNEL_TOTAL_INPUTS, samplesPerBlock);
    multiFeedbackBuffer.clear();
    multibuffer.clear();

    fdn.prepareDelayLine(sampleRate, MULTICHANNEL_TOTAL_INPUTS);
    

    for (int i = 0; i < DIFFUSION_STEPS; ++i) {
        diffuse[i].prepareDelayLine(sampleRate, MULTICHANNEL_TOTAL_INPUTS);

    }
    for (size_t i = 0; i < rands_array.size(); ++i) {
        int lower = (5000 / MULTICHANNEL_TOTAL_INPUTS)*i;
        int upper = (5000 / MULTICHANNEL_TOTAL_INPUTS)*(i+1);
        rands_array[i] = rand.getSystemRandom().nextInt(juce::Range<int>(lower, upper));
    }

    for (int channel = 0; channel < MULTICHANNEL_TOTAL_INPUTS; ++channel) {
        lowPassFilters[channel].setCoefficients(juce::IIRCoefficients::makeLowPass(sampleRate, 5000));
    }
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


    auto bufferSize = buffer.getNumSamples();;
    for (int channels = 0; channels < MULTICHANNEL_TOTAL_INPUTS; channels++) {
        int original_channel = channels % getTotalNumInputChannels();
        multibuffer.copyFromWithRamp(channels, 0, buffer.getReadPointer(original_channel), bufferSize, .8f, .8f);
    }
    auto mBufferSize = multibuffer.getNumSamples();
    
    
    


    multiFeedbackBuffer.clear();
    for (int i = 0; i < DIFFUSION_STEPS; ++i) {
        diffuse[i].fillDelayBuff(multibuffer, MULTICHANNEL_TOTAL_INPUTS);
        diffuse[i].setDelay(msToSamples(20)*i);
        diffuse[i].diffuseBuffer(multibuffer, rands_array, i);


        const float diffuseGain = 0.9 - (i * 0.1);
        for (int i = 0; i < MULTICHANNEL_TOTAL_INPUTS; ++i) {
            //lowPassFilters[i].processSamples(multibuffer.getWritePointer(i), mBufferSize);
            multiFeedbackBuffer.addFromWithRamp(i, 0, multibuffer.getReadPointer(i), bufferSize, diffuseGain, diffuseGain);
        }


    }


    fdn.fillDelayBuff(multiFeedbackBuffer, MULTICHANNEL_TOTAL_INPUTS);

    const auto RoomSizePID = static_cast<int>(params::PID::RoomSize);
    const auto delayNorm = parameters[RoomSizePID]->getValue();
    const auto delay = parameters[RoomSizePID]->getNormalisableRange().convertFrom0to1(delayNorm);
    fdn.setDelay(msToSamples(delay));



    const auto decayPID = static_cast<int>(params::PID::Decay);
    const auto decayNorm = parameters[decayPID]->getValue();
    const auto decay = parameters[decayPID]->getNormalisableRange().convertFrom0to1(decayNorm);
    fdn.setDecay(decay);

    const auto earlyRefPID = static_cast<int>(params::PID::EarlyReflections);
    const auto earlyRefNorm = parameters[earlyRefPID]->getValue();
    const auto earlyRef = parameters[earlyRefPID]->getNormalisableRange().convertFrom0to1(earlyRefNorm);

    auto* multidata = multiFeedbackBuffer.getArrayOfWritePointers();
    
    for (int i = 0; i < mBufferSize; ++i) {
        fdn.process(i, multidata, rands_array);
    }
    
    const auto og_channels = buffer.getNumChannels();
    for (int channel = 0; channel < MULTICHANNEL_TOTAL_INPUTS; ++channel)
    {
        
        int original_channel = channel % og_channels;
        
        
        buffer.copyFromWithRamp(original_channel, 0, multiFeedbackBuffer.getReadPointer(channel), bufferSize, 1.f, 1.f);
        buffer.addFromWithRamp(original_channel, 0, multibuffer.getReadPointer(channel), bufferSize, earlyRef, earlyRef);
        
    }
    fdn.updateWrite(buffer.getNumSamples());
    for (int i = 0; i < DIFFUSION_STEPS; ++i) {
        diffuse[i].updateWrite(mBufferSize);
    }
    
    /*write_pos += bufferSize;
    write_pos %= dBufferSize;
    diffuse.updateWritePos(mBufferSize);*/
    //buffer.applyGain(2);
}
float Reverb_Plugin_Colon_BracketAudioProcessor::msToSamples(float ms) {
    const auto sampleRate = getSampleRate();

    return (ms * sampleRate) / 1000;
}
void Reverb_Plugin_Colon_BracketAudioProcessor::readFromBuffer(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& dBuffer, int channel, int bufferSize, int dBufferSize) {

    auto readPosition = write_pos - (5000 + (3000*channel));

    const float gainDivisor = 4.f;

    if (readPosition < 0)
        readPosition += dBufferSize;

    auto* dataArr = dBuffer.getArrayOfWritePointers();
   
    /*for (int l = 0; l < buffer.getNumSamples(); ++l) {
        Householder<float, 4>::inPlace(dataArr, l);
    }*/

    
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

    
    /*for (int l = 0; l < buffer.getNumSamples(); ++l) {
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                inputMatrix(i, j) = dataArr[j][l];
            }
        }

        outputMatrix = inputMatrix * householderMatrix;

        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                dataArr[j][l] = outputMatrix(i, j);
            }
        }
    }*/

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
