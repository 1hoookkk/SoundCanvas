#include "CanvasComponent.h"

void CanvasComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::white);
    for (const auto& pt : strokes)
    {
        int x = static_cast<int>(pt.timeNorm * getWidth());
        int y = static_cast<int>((1.0f - pt.freqNorm) * getHeight());
        g.fillEllipse((float)x - 1, (float)y - 1, 2.0f, 2.0f);
    }
}

void CanvasComponent::mouseDrag(const juce::MouseEvent& e)
{
    float t = juce::jlimit(0.0f, 1.0f, (float)e.position.x / (float)getWidth());
    float f = juce::jlimit(0.0f, 1.0f, 1.0f - (float)e.position.y / (float)getHeight());

    strokes.push_back({ t, f });
    repaint();
}
