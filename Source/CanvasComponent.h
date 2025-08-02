#pragma once
#include <JuceHeader.h>

struct StrokePoint
{
    float timeNorm;   // [0,1]
    float freqNorm;   // [0,1]
};

class CanvasComponent : public juce::Component
{
public:
    void paint(juce::Graphics&) override;
    void mouseDrag(const juce::MouseEvent&) override;

    const std::vector<StrokePoint>& getStrokes() const { return strokes; }

private:
    std::vector<StrokePoint> strokes;
};
