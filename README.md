# Redux Reverb !

An algorithmic audio reverb implemented in C++17 using the JUCE framework, based on the reverb architecture proposed by Geraint Luff (Signalsmith) at the Audio Developer Conference 2021.

The project explores algorithmic reverberation, digital signal processing, and real-time audio software development, translating a published reverb design into a working audio application.

**Technologies**: C++17 · JUCE · Digital Signal Processing · Real-time Audio

## Outline
The basic structure of the reverb consists of a feedback delay loop and a diffuser
<img width="665" height="244" alt="image" src="https://github.com/user-attachments/assets/0237e576-96d9-4f3c-9214-c67a608d7345" />

The length of the delay specifies the decay time while the diffuser simulates the initial reverberation (ie the pre-delay, room size etc)
<img width="354" height="243" alt="image" src="https://github.com/user-attachments/assets/377fd7db-f6c6-4aad-9d06-1d79e179ad4d" />

The design splits the incoming signal into multiple duplicate channels, which are then diffused and delayed until brought back into stereo.

## Build
This project can easily be built using the <a href="https://juce.com/get-juce/">Projucer</a>. The project can be loaded and built by selecting VST3 as the active build target.

## Accompanying Video
https://www.youtube.com/watch?v=4YZqfjqW_F8
