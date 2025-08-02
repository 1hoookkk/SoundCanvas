// Source/GUI/PluginEditor.h
#pragma once

#include <JuceHeader.h>
#include "Core/Commands.h"
#include "Core/ForgeProcessor.h"
#include "Core/CanvasProcessor.h"
#include "Core/ParameterBridge.h"
#include "GUI/CanvasPanel.h"

class ARTEFACTAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::Button::Listener,
    public juce::Timer
{
public:
    explicit ARTEFACTAudioProcessorEditor(ARTEFACTAudioProcessor&);
    ~ARTEFACTAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void buttonClicked(juce::Button*) override;
    void timerCallback() override;

private:
    ARTEFACTAudioProcessor& processor;
    std::unique_ptr<CanvasPanel> canvasPanel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ARTEFACTAudioProcessorEditor)
};
