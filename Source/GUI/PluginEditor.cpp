#include "PluginEditor.h"
#include "Core/PluginProcessor.h"       // NOT "../PluginProcessor.h"
#include "ArtefactLookAndFeel.h"
#include "HeaderBarComponent.h"
#include "ForgePanel.h"
#include "../Core/Commands.h"          

//==============================================================================
ARTEFACTAudioProcessorEditor::ARTEFACTAudioProcessorEditor(ARTEFACTAudioProcessor& p)
    : juce::AudioProcessorEditor(p),
    audioProcessor(p)
{
    // Create look and feel first
    artefactLookAndFeel = std::make_unique<ArtefactLookAndFeel>();
    setLookAndFeel(artefactLookAndFeel.get());

    // Create UI components
    headerBar = std::make_unique<HeaderBarComponent>();
    forgePanel = std::make_unique<ForgePanel>(p);

    addAndMakeVisible(headerBar.get());
    addAndMakeVisible(forgePanel.get());

    // Add test button
    addAndMakeVisible(testButton);
    testButton.addListener(this);

    setSize(800, 600);
}

ARTEFACTAudioProcessorEditor::~ARTEFACTAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void ARTEFACTAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void ARTEFACTAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    const int headerH = 40;

    headerBar->setBounds(bounds.removeFromTop(headerH));

    // Place test button in top-right corner
    testButton.setBounds(bounds.removeFromTop(30).removeFromRight(100).reduced(5));

    forgePanel->setBounds(bounds);
}

void ARTEFACTAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &testButton)
    {
        // When the test button is clicked, push our test command to the queue.
        audioProcessor.pushCommandToQueue(Command(ForgeCommandID::Test));
        DBG("Test button clicked - command sent!");
    }
}