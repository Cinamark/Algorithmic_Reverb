# Redux Reverb !

An algorithmic audio reverb implemented in C++17 using the JUCE framework, based on the reverb architecture proposed by Geraint Luff (Signalsmith) at the Audio Developer Conference 2021.

The project explores algorithmic reverberation, digital signal processing, and real-time audio software development, translating a published reverb design into a working audio application.

## Technical Implementation

- **Language:** C++17
- **Framework:** JUCE
- **Architecture:** Feedback delay network with diffusion stages
- **Audio Processing:** Real-time digital signal processing
- **Plugin Format:** VST3

## Outline
The reverb architecture consists of a feedback delay network and a diffusion stage. The feedback network generates the decaying reverberation, while the diffusion stage creates a denser, smoother reflection pattern.<br>
<img width="354" height="243" alt="image" src="https://github.com/user-attachments/assets/377fd7db-f6c6-4aad-9d06-1d79e179ad4d" />
<img width="665" height="244" alt="image" src="https://github.com/user-attachments/assets/0237e576-96d9-4f3c-9214-c67a608d7345" />
<br>

The feedback delay network (FDN) determines the time characteristics of the reverberation, with emphasis on its decay behaviour. Moreover, the diffusion stage helps control the density and distribution of reflections, contributing to the perceived characteristics of the "room".

The design splits the incoming signal into multiple duplicate channels, which are then diffused and delayed until brought back into stereo.

## Building

### Requirements
- C++17 compatible compiler
- JUCE
- Projucer

### Instructions

1. Clone the repository.
2. Open the `.jucer` project file using <a href="https://juce.com/get-juce/">Projucer</a>.
3. Select your preferred IDE exporter or build configuration.
4. Set VST3 as the active plugin format.
5. Save the project and open the generated project in your IDE.
6. Build the project.

## Accompanying Video

A demonstration of the implemented reverb, showcasing its audio processing and resulting sound.

[Watch the demonstration on YouTube](https://www.youtube.com/watch?v=4YZqfjqW_F8)

## References & Acknowledgements

This project is based on the reverb architecture presented by Geraint Luff (Signalsmith Audio).

- **Original ADC21 presentation:** [Let's Write a Reverb – Geraint Luff](https://www.youtube.com/watch?v=6ZK2Goiyotk&t=2127s)
- **Technical article:** [Let's Write a Reverb – Signalsmith Audio](https://signalsmith-audio.co.uk/writing/2021/lets-write-a-reverb/)

The original presentation and article provide the theoretical foundation for the feedback delay network and diffusion architecture implemented in this project.
