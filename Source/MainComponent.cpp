#include "MainComponent.h"

MainComponent::MainComponent()
{
    addAndMakeVisible(canvas);
    addAndMakeVisible(renderButton);
    renderButton.addListener(this);
    setSize(800, 600);
}

void MainComponent::resized()
{
    canvas.setBounds(10, 10, getWidth() - 20, getHeight() - 60);
    renderButton.setBounds(10, getHeight() - 40, 120, 30);
}

void MainComponent::buttonClicked(juce::Button* b)
{
    if (b == &renderButton)
    {
        juce::AudioBuffer<float> buffer;
        SoundRenderer::renderFromCanvas(canvas.getStrokes(), buffer, 44100, 2.0f);

        juce::File outputFile = juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
            .getChildFile("spectral_output.wav");

        juce::FileOutputStream stream(outputFile);
        if (stream.openedOk())
        {
            juce::WavAudioFormat wavFormat;
            std::unique_ptr<juce::AudioFormatWriter> writer = wavFormat.createWriterFor(&stream, 44100,
                1, 16, {}, 0);
            if (writer != nullptr)
                writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
        }
    }
}
