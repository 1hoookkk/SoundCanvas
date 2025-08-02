// GUI/ForgePanel.h
#pragma once

#include <JuceHeader.h>
#include <vector>
#include <memory>

// Forward declarations
class ARTEFACTAudioProcessor;
class SampleSlotComponent;

class ForgePanel : public juce::Component
{
public:
    explicit ForgePanel(ARTEFACTAudioProcessor& processorToUse);
    ~ForgePanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    ARTEFACTAudioProcessor& processor;

    static constexpr int numSlots = 8;
    std::vector<std::unique_ptr<SampleSlotComponent>> sampleSlots;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ForgePanel)
};