#pragma once
#include <JuceHeader.h>

class HeaderBarComponent : public juce::Component
{
public:
    HeaderBarComponent();
    ~HeaderBarComponent() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderBarComponent)
};
