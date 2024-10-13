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

    void setDelay(int delay) {
        m_delayLines.setDelay(delay);
    }

    void fillDelayBuff(juce::AudioBuffer<float>& buff, int channels) {
        m_delayLines.fillCBuff(buff);
    }
    void process(int index, float* const* buffData, std::array<int, NumChannels> delayOffset) { // called per sample, with something like a std::array<float, NumFdnChannels>
        // delay line outs
        for (size_t i = 0; i < NumChannels; ++i) {
            m_scratchBuffer[i] = m_feedbackBuffer[i] = m_delayLines.popSample(i, index, delayOffset[i]); // grab the output of the delay line - scratch buffer is going to be our actual output, feedback buffer is going to be where we apply the mix matrix and decay, for writing back into the delay line
        }
        Householder<float, NumChannels>::inPlace(m_feedbackBuffer.data()); // shuffle
        //someSimdMathHelper::multiply(m_feedbackBuffer, decay); // apply decay - if you're not feeling fancy or dont have access to a handy simd way to do this, just apply it in the loop below instead..
        // Cool, time to write to the delay lines..;
        for (size_t i = 0; i < NumChannels; ++i) {
            // (Feedback has already been applied)
            const float delayLineInput = (buffData[i][index] + m_feedbackBuffer[i]) * .5f;

            m_delayLines.pushSample(delayLineInput, i, index);
            // And then throw the output of the delay (unfeedbacked etc) into our currentSample
            
            buffData[i][index] += m_scratchBuffer[i];
        }
    }

    void updateWrite(int size) {
        m_delayLines.updateWritePos(size);
    }
private:
    DelayLine m_delayLines; // One single channel delay line per FDN channel
    std::array<float, NumChannels> m_feedbackBuffer, m_scratchBuffer;
};

