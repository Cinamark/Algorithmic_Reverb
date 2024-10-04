/*
  ==============================================================================

    Diffuser.h
    Created: 4 Aug 2024 9:36:31am
    Author:  Simeon

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

struct Diffuser {
    void prepare(int sampleRate, int channels, int samplesExpected) {
        auto cLength = static_cast<int>(sampleRate) * 2;
        circularBuffer.setSize(channels, cLength);
    }

    /*void mapToMultichannel(juce::AudioBuffer<float>& Originalbuff, juce::AudioBuffer<float>& Multibuff) {
        auto total_channels = circularBuffer.getNumChannels();
        auto size = Originalbuff.getNumSamples();
        auto cSize = circularBuffer.getNumSamples();
        for (int channels = 0; channels < total_channels; channels++) {
            int original_channel = channels % total_channels;
           Multibuff.copyFromWithRamp(channels, 0, Originalbuff.getReadPointer(original_channel), size, .5f, .5f);
        }
    }*/

    void fillCBuff(juce::AudioBuffer<float>& buff) {

        auto total_channels = circularBuffer.getNumChannels();
        auto size = buff.getNumSamples();
        auto cSize = circularBuffer.getNumSamples();

        for (int channels = 0; channels < total_channels; channels++) {
            auto* channelData = buff.getWritePointer(channels);
            if (cSize > size + write_pos)
            {
                circularBuffer.copyFromWithRamp(channels, write_pos, channelData, size, .1f, .1f);
            }
            else
            {
                auto numSamplesToEnd = cSize - write_pos;
                circularBuffer.copyFromWithRamp(channels, write_pos, channelData, numSamplesToEnd, .1f, .1f);

                auto numSamplesAtStart = size - numSamplesToEnd;
                circularBuffer.copyFromWithRamp(channels, 0, channelData + numSamplesToEnd, numSamplesAtStart, .1f, .1f);
            }

        }
        

    }

    void readCBuff(juce::AudioBuffer<float>& buffer, int channels, int bufferSize, int dBufferSize) {

        jassert(circularBuffer.getNumSamples() == (buffer.getNumSamples() * 2));
        auto readPosition = write_pos - (3000 * channels + 1);

        const float gainDivisor = 4.f;

        if (readPosition < 0)
            readPosition += dBufferSize;
 
        for (int channel = 0; channels < channels; channel++) {
            if (readPosition + bufferSize < dBufferSize)
            {
                buffer.addFromWithRamp(channel  , 0, circularBuffer.getReadPointer(channel, readPosition), bufferSize, gainDivisor, gainDivisor);
            }

            else {
                auto numSamplesToEnd = dBufferSize - readPosition;
                buffer.addFromWithRamp(channel, 0, circularBuffer.getReadPointer(channel, readPosition), numSamplesToEnd, gainDivisor, gainDivisor);

                auto numSamplesAtStart = bufferSize - numSamplesToEnd;
                buffer.addFromWithRamp(channel, numSamplesToEnd, circularBuffer.getReadPointer(channel, 0), numSamplesAtStart, gainDivisor, gainDivisor);
            }

        }
        

    }

    void copyCBuff(juce::AudioBuffer<float>& buffer, int bufferSize, int dBufferSize) {

        //jassert(circularBuffer.getNumSamples() == (buffer.getNumSamples() * 2));
        
        auto channels = buffer.getNumChannels();
        const float gainDivisor = 4.f;

        for (int channel = 0; channel < channels; channel++) {

            auto readPosition = write_pos - (10000 * channels);
            if (readPosition < 0)
                readPosition += dBufferSize;

            if (readPosition + bufferSize < dBufferSize)
            {
                buffer.addFromWithRamp(channel  , 0, circularBuffer.getReadPointer(channel, readPosition), bufferSize, gainDivisor, gainDivisor);
            }

            else {
                auto numSamplesToEnd = dBufferSize - readPosition;
                buffer.addFromWithRamp(channel, 0, circularBuffer.getReadPointer(channel, readPosition), numSamplesToEnd, gainDivisor, gainDivisor);

                auto numSamplesAtStart = bufferSize - numSamplesToEnd;
                buffer.addFromWithRamp(channel, numSamplesToEnd, circularBuffer.getReadPointer(channel, 0), numSamplesAtStart, gainDivisor, gainDivisor);
            }

        }
        

    }
    
    void diffuseBuffer(juce::AudioBuffer<float>& buffer) {

        auto size = buffer.getNumSamples();
        auto delaySize = circularBuffer.getNumSamples();
        fillCBuff(buffer);
        copyCBuff(buffer, size, delaySize);

        updateWritePos(size, delaySize);

    }
    void updateWritePos(int size, int delaySize) {
        write_pos += size;
        write_pos %= delaySize;
    }
    juce::AudioBuffer<float> circularBuffer;
    int write_pos{ 0 };
};