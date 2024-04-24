# SelfModulator
SelfModulator is a VST3 audio plugin intended to be used in your Digital Audio Workstation (DAW) of choice. This plugin implements Figure 5. in this paper: https://www.music.mcgill.ca/~ich/research/misc/papers/cr1154.pdf.

## Build Instructions
1) Open SelfModulator.jucer in Projucer. Download Projucer at https://juce.com/get-juce/
2) Select exporter at the top (Visual Studio for Windows, Xcode for Mac). This plugin has only been tested on Windows.
    ![alt text](image.png | width=100)

    More export targets can be added in the "Exporters" panel with the "+" button.
    ![alt text](image-1.png | height=100)

3) Build the project in your IDE!

## Installation Instructions
1) VST3 output will be in the Builds folder. (Builds\VisualStudio2022\x64\Debug\VST3 for Visual Studio).
2) Drag SelfModulator.dll or SelfModulator.vst3 into your Steinberg VST3 folder and open up your DAW.