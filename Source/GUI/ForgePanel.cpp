// GUI/ForgePanel.cpp
#include "ForgePanel.h"
#include "ArtefactLookAndFeel.h"
#include "SampleSlotComponent.h"
#include "Core/PluginProcessor.h"


ForgePanel::ForgePanel(ARTEFACTAudioProcessor& processorToUse)
    : processor(processorToUse)
{
    // Create sample slots
    for (int i = 0; i < numSlots; ++i)
    {
        auto slot = std::make_unique<SampleSlotComponent>(processor, i);
        addAndMakeVisible(slot.get());
        sampleSlots.push_back(std::move(slot));
    }
}

void ForgePanel::paint(juce::Graphics& g)
{
    g.fillAll(ArtefactLookAndFeel::kPanelBackground);

    // Draw panel title
    g.setColour(ArtefactLookAndFeel::kTextColour);
    g.setFont(14.0f);
    g.drawText("FORGE", getLocalBounds().removeFromTop(30),
        juce::Justification::centred);
}

void ForgePanel::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(30); // Title area

    const int slotHeight = 40;
    const int expandedHeight = 100;
    const int padding = 2;

    int yPos = bounds.getY();

    for (auto& slot : sampleSlots)
    {
        int height = slot->getHeight() > slotHeight ? expandedHeight : slotHeight;
        slot->setBounds(bounds.getX(), yPos, bounds.getWidth(), height);
        yPos += height + padding;
    }
}