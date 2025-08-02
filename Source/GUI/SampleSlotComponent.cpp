// Source/GUI/SampleSlotComponent.cpp
#include "SampleSlotComponent.h"
#include "ArtefactLookAndFeel.h"
#include "Core/PluginProcessor.h"

// …

void SampleSlotComponent::resized()
{
    if (isExpanded)
    {
        const int knob = 40, gap = 60;
        pitchSlider.setBounds(20, 50, knob, knob);
        speedSlider.setBounds(20 + gap, 50, knob, knob);
        volumeSlider.setBounds(20 + gap * 2, 50, knob, knob);
        driveSlider.setBounds(20 + gap * 3, 50, knob, knob);
        crushSlider.setBounds(20 + gap * 4, 50, knob, knob);
        syncButton.setBounds(getWidth() - 60, 50, 50, 25);

        for (juce::Component* c : {
            static_cast<juce::Component*>(&pitchSlider),
            static_cast<juce::Component*>(&speedSlider),
            static_cast<juce::Component*>(&volumeSlider),
            static_cast<juce::Component*>(&driveSlider),
            static_cast<juce::Component*>(&crushSlider),
            static_cast<juce::Component*>(&syncButton)
            })
            c->setVisible(true);
    }
    else
    {
        for (juce::Component* c : {
            static_cast<juce::Component*>(&pitchSlider),
            static_cast<juce::Component*>(&speedSlider),
            static_cast<juce::Component*>(&volumeSlider),
            static_cast<juce::Component*>(&driveSlider),
            static_cast<juce::Component*>(&crushSlider),
            static_cast<juce::Component*>(&syncButton)
            })
            c->setVisible(false);
    }
}

void SampleSlotComponent::mouseDown(const juce::MouseEvent& e)
{
    auto& v = processor.getForgeProcessor().getVoice(slotIndex);

    if (e.mods.isRightButtonDown())
    {
        isExpanded = !isExpanded;
        setSize(getWidth(), isExpanded ? 100 : 40);

        for (juce::Component* c : {
            static_cast<juce::Component*>(&pitchSlider),
            static_cast<juce::Component*>(&speedSlider),
            static_cast<juce::Component*>(&volumeSlider),
            static_cast<juce::Component*>(&driveSlider),
            static_cast<juce::Component*>(&crushSlider),
            static_cast<juce::Component*>(&syncButton)
            })
            c->setVisible(isExpanded);

        resized();
        if (auto* p = getParentComponent()) p->resized();
    }
    else if (v.hasSample())
    {
        processor.pushCommandToQueue(
            Command(v.isActive() ? ForgeCommandID::StopPlayback
                : ForgeCommandID::StartPlayback,
                slotIndex));
    }
    repaint();
}
