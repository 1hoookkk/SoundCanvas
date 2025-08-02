#pragma once

#include <JuceHeader.h>        // instead of juce_gui_basics/juce_gui_basics.h
#include "Core/PluginProcessor.h" // instead of just a forward declaration
#include <memory>

class ARTEFACTAudioProcessor;

//==============================================================================
//  ░█▀▄░█▀█░█▀▀░█░░░█▀█░█▀█
//  ░█░█░█▀█░█░░░█░░░█▀█░█░█
//  ░▀▀░░▀░▀░▀▀▀░▀▀▀░▀░▀░▀░▀
class SampleSlotComponent : public juce::Component,
    public juce::FileDragAndDropTarget,
    public juce::Timer
{
public:
    SampleSlotComponent(ARTEFACTAudioProcessor& processor, int slotIndex);
    ~SampleSlotComponent() override;

    // ─────────────────────────────────────────── Component
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

    // ─────────────────────────────────────────── DnD
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    // ─────────────────────────────────────────── Timer
    void timerCallback() override;

private:
    using SliderAttach = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttach = juce::AudioProcessorValueTreeState::ButtonAttachment;

    ARTEFACTAudioProcessor& processor;
    const int               slotIndex;

    // UI
    juce::Slider   pitchSlider, speedSlider, volumeSlider, driveSlider, crushSlider;
    juce::TextButton syncButton{ "SYNC" };

    // Attachments
    std::unique_ptr<SliderAttach> pitchAttachment, speedAttachment, volumeAttachment,
        driveAttachment, crushAttachment;
    std::unique_ptr<ButtonAttach> syncAttachment;

    // Waveform display
    juce::Path waveformPath;
    float      playheadPosition = 0.0f;

    // State
    bool isExpanded = false;

    // Helpers
    void updateWaveformPath();
    void updateFromProcessor();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleSlotComponent)
};
