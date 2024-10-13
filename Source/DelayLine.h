/*
  ==============================================================================

    DelayLine.h
    Created: 7 Oct 2024 6:54:13am
    Author:  trdte

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

//Stuart
class DelayLine {

public:
    void prepare(int sampleRate, int channels) {
        cBuffSize = sampleRate * 2;
        cBuffer.setSize(channels,cBuffSize);
        cBuffer.clear();
    }

    void fillCBuff(juce::AudioBuffer<float>& buffer) {
        auto size = buffer.getNumSamples();
        auto cSize = cBuffer.getNumSamples();
        auto channels = buffer.getNumChannels();
        jassert(channels = cBuffer.getNumChannels());
        for (int current_channel = 0; current_channel < channels; ++current_channel) {
            auto* channelData = buffer.getReadPointer(current_channel);
            if (cSize > size + write_pos)
            {
                cBuffer.copyFromWithRamp(current_channel, write_pos, channelData, size, 1.f, 1.1f);
            }
            else
            {   
                auto numSamplesToEnd = cSize - write_pos;
                cBuffer.copyFromWithRamp(current_channel, write_pos, channelData, numSamplesToEnd, 1.f, 1.f);

                auto numSamplesAtStart = size - numSamplesToEnd;
                cBuffer.copyFromWithRamp(current_channel, 0, channelData + numSamplesToEnd, numSamplesAtStart, 1.f, 1.f);
            }
        }

        

        
    }
    void updateWritePos(int size) {
        auto cSize = cBuffer.getNumSamples();
        write_pos += size;
        write_pos %= cSize;
    }
    void pushSample(float sample, int channel, int index) {
        auto* data = cBuffer.getWritePointer(channel);
        int pushWrite = write_pos + index;
        pushWrite %= cBuffer.getNumSamples();
        data[pushWrite] = sample;

    }
    float popSample(int channel, int index, int offset) {
        auto* data = cBuffer.getReadPointer(channel);
        int read_pos = setReadPos(m_delay, channel, index, offset);
        return data[read_pos];
    }

    void setDelay(int delay){
        m_delay = delay;
        

    }

   

    int setReadPos(int delay, int channel, int index, int offset) {
        auto totalChannels = cBuffer.getNumChannels();
        int scratch_rpos = write_pos - (delay / (totalChannels-channel)) + index;

        if (scratch_rpos < 0) {
            scratch_rpos += cBuffSize;
        }
        
        scratch_rpos += offset;
        scratch_rpos %= cBuffer.getNumSamples();
        return scratch_rpos;
    }
private:
    juce::AudioBuffer<float> cBuffer;
    int cBuffSize;
    int write_pos{ 0 };
    int m_delay{ 0 };
};