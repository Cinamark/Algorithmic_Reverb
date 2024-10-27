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
        float read_pos = setReadPos(m_delay, channel, index, offset);
         if (isInteger(read_pos)) {
            int read_index = static_cast<int>(read_pos) % cBuffer.getNumSamples();
            return data[read_index];
        }
        else {
            int floor = static_cast<int>(read_pos) % cBuffer.getNumSamples();
            int ceil = (floor + 1) % cBuffer.getNumSamples();

            float fraction = read_pos - floor;

            return (data[floor] * (1.f - fraction)) + (data[ceil] * fraction);
        }
    }
    float setReadPos(float delay, int channel, int index, int offset) {
        auto totalChannels = cBuffer.getNumChannels();
        float scratch_rpos = write_pos - (((delay / static_cast<float>(totalChannels)) * channel + 0) + offset) + index;

        if (scratch_rpos < 0) {
            scratch_rpos += cBuffSize;
        }

        return scratch_rpos;
    }
    void setDelay(float delay){
        m_delay = delay;
        

    }


    bool isInteger(float N)
    {
        int X = N;

        float temp2 = N - X;

        if (temp2 > 0) {
            return false;
        }
        return true;
    }


private:
    juce::AudioBuffer<float> cBuffer;
    int cBuffSize;
    int write_pos{ 0 };
    float m_delay{ 0 };
};

class AllpassDelayLine {

public:
    void prepare(int sampleRate, int channels) {
        cBuffSize = sampleRate * 2;
        cBuffer.setSize(channels, cBuffSize);
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
                cBuffer.copyFromWithRamp(current_channel, write_pos, channelData, size, .95f, .951f);
            }
            else
            {
                auto numSamplesToEnd = cSize - write_pos;
                cBuffer.copyFromWithRamp(current_channel, write_pos, channelData, numSamplesToEnd, .95f, .95f);

                auto numSamplesAtStart = size - numSamplesToEnd;
                cBuffer.copyFromWithRamp(current_channel, 0, channelData + numSamplesToEnd, numSamplesAtStart, .95f, .95f);
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
        float read_pos = setReadPos(m_delay, channel, index, offset);
        if (isInteger(read_pos)) {
            int read_index = static_cast<int>(read_pos) % cBuffer.getNumSamples();
            return data[read_index];
        }
        else {
            int floor = static_cast<int>(read_pos) % cBuffer.getNumSamples();
            int ceil = (floor + 1) % cBuffer.getNumSamples();

            float fraction = read_pos - floor;

            return (data[floor] * (1.f - fraction)) + (data[ceil] * fraction);
        }
    }
    float setReadPos(float delay, int channel, int index, int offset) {
        auto totalChannels = cBuffer.getNumChannels();
        float scratch_rpos = write_pos - (((delay / static_cast<float>(totalChannels)) * (channel + 1)) + offset) + index;

        if (scratch_rpos < 0) {
            scratch_rpos += cBuffSize;
        }

        return scratch_rpos;
    }

    void setDelay(float delay) {
        m_delay = delay;


    }


    bool isInteger(float N)
    {
        int X = N;

        float temp2 = N - X;

        if (temp2 > 0) {
            return false;
        }
        return true;
    }
    
private:
    juce::AudioBuffer<float> cBuffer;
    int cBuffSize = 0;
    int write_pos{ 0 };
    float m_delay{ 0 };
    std::array<float, 8> delayTimes;
};