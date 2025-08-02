// Source/PluginProcessor.cpp
#include "PluginProcessor.h"
#include "GUI/PluginEditor.h"

bool ARTEFACTAudioProcessor::pushCommandToQueue(const Command& newCommand)
{
    int start, end;
    abstractFifo.prepareToWrite(1, start, end);
    if (start != end)
    {
        commandFIFO[start] = newCommand;
        abstractFifo.finishedWrite(1);
        return true;
    }
    return false;
}

void ARTEFACTAudioProcessor::processNextCommand()
{
    if (abstractFifo.getNumReady() == 0)
        return;

    int start, end;
    abstractFifo.prepareToRead(1, start, end);
    if (start != end)
    {
        const auto& cmd = commandFIFO[start];
        switch (cmd.id)
        {
        case ForgeCommandID::StartPlayback:
            forgeProcessor.getVoice(cmd.intParam).start();                break;
        case ForgeCommandID::StopPlayback:
            forgeProcessor.getVoice(cmd.intParam).stop();                 break;
        case ForgeCommandID::LoadSample:
            forgeProcessor.getVoice(cmd.intParam).loadFromFile(cmd.stringParam); break;
        case ForgeCommandID::SetPitch:
            forgeProcessor.getVoice(cmd.intParam).setPitch(cmd.floatParam);       break;
        case ForgeCommandID::SetSpeed:
            forgeProcessor.getVoice(cmd.intParam).setSpeed(cmd.floatParam);       break;
        case ForgeCommandID::SetVolume:
            forgeProcessor.getVoice(cmd.intParam).setVolume(cmd.floatParam);      break;
        case ForgeCommandID::SetDrive:
            forgeProcessor.getVoice(cmd.intParam).setDrive(cmd.floatParam);       break;
        case ForgeCommandID::SetCrush:
            forgeProcessor.getVoice(cmd.intParam).setCrush(cmd.floatParam);       break;
        case ForgeCommandID::SetSyncMode:
            forgeProcessor.getVoice(cmd.intParam).setSyncMode(cmd.boolParam);     break;
        default: break;
        }
    }
    abstractFifo.finishedRead(1);
}

void ARTEFACTAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    for (int i = 0; i < fifoSize; ++i)
        processNextCommand();

    // host BPM, mode switch, then:
    if (currentMode == ProcessingMode::Canvas)
        canvasProcessor.processBlock(buffer);
    else if (currentMode == ProcessingMode::Forge)
        forgeProcessor.processBlock(buffer, midi);
    else
    {
        juce::AudioBuffer<float> tmp(buffer.getNumChannels(), buffer.getNumSamples());
        canvasProcessor.processBlock(tmp);
        forgeProcessor.processBlock(buffer, midi);
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.addFrom(ch, 0, tmp, ch, 0, buffer.getNumSamples(), 0.5f);
    }
}
