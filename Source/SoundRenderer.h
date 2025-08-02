#pragma once
#include <JuceHeader.h>
#include "CanvasComponent.h"

namespace SoundRenderer
{
    void renderFromCanvas(const std::vector<StrokePoint>& strokes,
        juce::AudioBuffer<float>& buffer,
        int sampleRate,
        float durationSeconds);
}
