#include "SoundRenderer.h"
#include <cmath>

void SoundRenderer::renderFromCanvas(const std::vector<StrokePoint>& strokes,
    juce::AudioBuffer<float>& buffer,
    int sampleRate,
    float durationSeconds)
{
    int totalSamples = static_cast<int>(sampleRate * durationSeconds);
    buffer.setSize(1, totalSamples);
    buffer.clear();

    for (const auto& pt : strokes)
    {
        int startSample = static_cast<int>(pt.timeNorm * totalSamples);
        float freq = juce::jmap(pt.freqNorm, 50.0f, 5000.0f);
        float amp = 0.3f;

        for (int i = 0; i < 2000; ++i)
        {
            int idx = startSample + i;
            if (idx >= totalSamples) break;
            float sample = amp * std::sin(2.0f * juce::MathConstants<float>::pi * freq * i / (float)sampleRate);
            buffer.setSample(0, idx, buffer.getSample(0, idx) + sample);
        }
    }
}
