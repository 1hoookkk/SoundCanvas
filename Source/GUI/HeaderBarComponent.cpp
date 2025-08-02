#include "HeaderBarComponent.h"

HeaderBarComponent::HeaderBarComponent()
{
    // Any future subcomponents or setup here.
}

void HeaderBarComponent::paint(juce::Graphics& g)
{
    // Fill the header background with a strong accent or background color
    g.setColour(juce::Colour::fromString("FF212E38"));
    g.fillRect(getLocalBounds());

    // Plugin name, centered
    g.setColour(juce::Colour::fromString("FF50A0F0")); // Accent blue
    g.setFont(juce::Font("Silkscreen", 18.0f, juce::Font::bold));
    g.drawFittedText("ARTEFACT", getLocalBounds(), juce::Justification::centred, 1);
}

void HeaderBarComponent::resized()
{
    // No child components yet
}
