/*
  ==============================================================================

    Params.h
    Created: 10 Oct 2024 5:57:33pm
    Author:  trdte

  ==============================================================================
*/

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace params {
    using APVTS = juce::AudioProcessorValueTreeState;
    using Layout = APVTS::ParameterLayout;
    using RAP = juce::RangedAudioParameter;
    using APP = juce::AudioProcessorParameter;
    using String = juce::String;
    
    enum class PID {
        RoomSize,
        Decay,
        EarlyReflections,
        PreResample,
        FeedbackResample,
        PostResample,
        DryWet,
        NumParams
    };

    static constexpr int NumParams = static_cast<int>(PID::NumParams);

    inline String toString(PID pid) {
        switch (pid)
        {
        case PID::RoomSize:
            return "Room Size";
        case PID::Decay:
            return "Decay";
        case PID::EarlyReflections:
            return "Early Reflections";
        case PID::PreResample:
            return "Pre_Down Sample";
        case PID::FeedbackResample:
            return "Feedback_Down Sample";
        case PID::PostResample:
            return "Post_Down Sample";
        case PID::DryWet:
            return "Dry_Wet";
        default:
            return "Unkown";
        }
    }

    inline String toID(const String& name) {
        return name.toLowerCase().removeCharacters(" ");
    }

    inline String toID(PID pid) {
        return toID(toString(pid));
    }
}