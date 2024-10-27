/*
  ==============================================================================

    FDN.h
    Created: 7 Oct 2024 9:12:30am
    Author:  trdte

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "DelayLine.h"

template<typename Sample, int size>
class Householder {
    static constexpr Sample multiplier{ -2.0 / size };
public:
    static void inPlace(Sample* arr) {
        double sum = 0;
        for (int i = 0; i < size; ++i) {
            sum += arr[i];
        }

        sum *= multiplier;

        for (int i = 0; i < size; ++i) {
            arr[i] += sum;
        }
    };
};

template<size_t NumChannels>
class FDN {
public:

    void prepareDelayLine(int sampleRate, int channels) {
        m_delayLines.prepare(sampleRate, channels);
        
    }

    void setDelay(float delay) {
        m_delayLines.setDelay(delay);
    }

    void fillDelayBuff(juce::AudioBuffer<float>& buff, int channels) {
        m_delayLines.fillCBuff(buff);
    }
    void process(int index, float* const* buffData, std::array<int, NumChannels> delayOffset) { 
        for (size_t i = 0; i < NumChannels; ++i) {
            m_scratchBuffer[i] = m_feedbackBuffer[i] = m_delayLines.popSample(i, index, delayOffset[i]); // grab the output of the delay line - scratch buffer is going to be our actual output, feedback buffer is going to be where we apply the mix matrix and decay, for writing back into the delay line
        }

        Householder<float, NumChannels>::inPlace(m_feedbackBuffer.data());

        for (size_t i = 0; i < NumChannels; ++i) {

            const float delayLineInput = (buffData[i][index] + m_feedbackBuffer[i]) * m_decay;

            m_delayLines.pushSample(delayLineInput, i, index);
            
            buffData[i][index] = m_scratchBuffer[i];
        }
    }

    void updateWrite(int size) {
        m_delayLines.updateWritePos(size);
    }

    void setDecay(float decay) {
        m_decay = decay;
    }
private:
    AllpassDelayLine m_delayLines; // One single channel delay line per FDN channel
    std::array<float, NumChannels> m_feedbackBuffer, m_scratchBuffer;
    float m_decay;
};

