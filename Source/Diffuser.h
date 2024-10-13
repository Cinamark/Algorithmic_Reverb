/*
  ==============================================================================

    Diffuser.h
    Created: 4 Aug 2024 9:36:31am
    Author:  Simeon

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "DelayLine.h"

template<typename Sample, int size>
class Hadamard {
public:
    static inline void recursiveUnscaled(Sample* data) {
        if (size <= 1) return;
        constexpr int hSize = size / 2;

        // Two (unscaled) Hadamards of half the size
        Hadamard<Sample, hSize>::recursiveUnscaled(data);
        Hadamard<Sample, hSize>::recursiveUnscaled(data + hSize);

        // Combine the two halves using sum/difference
        for (int i = 0; i < hSize; ++i) {
            double a = data[i];
            double b = data[i + hSize];
            data[i] = (a + b);
            data[i + hSize] = (a - b);
        }
    }

    static inline void inPlace(Sample* data) {
        recursiveUnscaled(data);

        Sample scalingFactor = std::sqrt(1.0 / size);
        for (int c = 0; c < size; ++c) {
            data[c] *= scalingFactor;
        }
    }
};
template<size_t NumChannels>
class Diffuser {
public:
    void prepareDelayLine(int sampleRate, int channels, int delay) {
        delayLine.prepare(sampleRate, channels);
        
    }

    void setDelay(int delay) {
        delayLine.setDelay(delay);
    }

    void diffuseBuffer(juce::AudioBuffer<float>& buffer, std::array<int, NumChannels> delayOffset) {
        auto size = buffer.getNumSamples();
        auto* data = buffer.getArrayOfWritePointers();
        int channels = buffer.getNumChannels();
        for (int index = 0; index < size; index++) {
            for (size_t i = 0; i < channels; ++i) {
                m_scratchBuffer[i] = m_feedbackBuffer[i] = delayLine.popSample(i, index, delayOffset[i]);
            }

            for (size_t i = 0; i < channels; i++) {
                if (isPrime(i)) {
                    m_feedbackBuffer[i] *= -1;
                }
                
            }

            Hadamard<float, NumChannels>::inPlace(m_feedbackBuffer.data());


            for (size_t i = 0; i < channels; ++i) {
                

                data[i][index] = m_feedbackBuffer[i];
            }
        }
        

    }

    void fillDelayBuff(juce::AudioBuffer<float>& buff, int channels) {
        delayLine.fillCBuff(buff);
    }
    
    bool isPrime(int num) {

        if (num == 0 || num == 1) {
            return false;
        }
        for (int i = 1; i < num; i++) {
            if (num % i == 0 && i != 1) {
                return false;
            }
        }

        return true;
    }
    void updateWrite(int size) {
        delayLine.updateWritePos(size);
    }

private: 
    DelayLine delayLine;
    std::array<float, NumChannels> m_feedbackBuffer, m_scratchBuffer;
};