#pragma once
#include <JuceHeader.h>
#include "CanvasComponent.h"
#include "SoundRenderer.h"

class MainComponent : public juce::Component,
    public juce::Button::Listener
{
public:
    MainComponent();
    void resized() override;
    void buttonClicked(juce::Button* b) override;

private:
    CanvasComponent canvas;
    juce::TextButton renderButton{ "Render Audio" };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
