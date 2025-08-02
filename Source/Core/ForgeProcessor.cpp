#include "Core/ForgeProcessor.h"

//==============================================================================
ForgeProcessor::ForgeProcessor()
{
    formatManager.registerBasicFormats();
    // voices[] already default-constructed in std::array – no push_back / clear
}

ForgeProcessor::~ForgeProcessor() = default;

//------------------------------------------------------------------------------
void ForgeProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    for (auto& v : voices)
        v.prepare(sampleRate, samplesPerBlock);
}

//------------------------------------------------------------------------------
void ForgeProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer&)
{
    for (auto& v : voices)
        v.process(buffer, 0, buffer.getNumSamples());
}

//------------------------------------------------------------------------------
void ForgeProcessor::loadSampleIntoSlot(int slotIdx, const juce::File& file)
{
    if (slotIdx < 0 || slotIdx >= (int)voices.size() || !file.existsAsFile())
        return;

    if (auto* r = formatManager.createReaderFor(file))
    {
        juce::AudioBuffer<float> tmp((int)r->numChannels,
            (int)r->lengthInSamples);
        r->read(&tmp, 0, tmp.getNumSamples(), 0, true, true);
        voices[(size_t)slotIdx].setSample(std::move(tmp), 120.0);
        delete r;
    }
}

//------------------------------------------------------------------------------
ForgeVoice& ForgeProcessor::getVoice(int index)
{
    jassert(isPositiveAndBelow(index, (int)voices.size()));
    return voices[(size_t)index];
}

//------------------------------------------------------------------------------
void ForgeProcessor::setHostBPM(double bpm)
{
    hostBPM = (float)bpm;
    for (auto& v : voices)
        v.setHostBPM(bpm);
}
