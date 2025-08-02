// Source/PluginProcessor.h
#pragma once

#include <JuceHeader.h>
#include "Core/Commands.h"
#include "Core/ForgeProcessor.h"
#include "Core/PaintEngine.h"
#include "Core/ParameterBridge.h"

class ARTEFACTAudioProcessor : public juce::AudioProcessor,
    public juce::AudioProcessorValueTreeState::Listener
{
public:
    ARTEFACTAudioProcessor();
    ~ARTEFACTAudioProcessor() override;

    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

    bool pushCommandToQueue(const Command& newCommand);

    void parameterChanged(const juce::String&, float) override;
    
    // Accessors for GUI
    ForgeProcessor& getForgeProcessor() { return forgeProcessor; }
    PaintEngine& getPaintEngine() { return paintEngine; }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts;

    ForgeProcessor  forgeProcessor;
    PaintEngine paintEngine;
    ParameterBridge parameterBridge;

    enum class ProcessingMode { Forge = 0, Canvas, Hybrid };
    ProcessingMode currentMode = ProcessingMode::Forge;

    static constexpr int fifoSize = 256;
    juce::AbstractFifo             abstractFifo{ fifoSize };
    std::array<Command, fifoSize>  commandFIFO;
    void processNextCommand();
    void processForgeCommand(const Command& cmd);
    void processPaintCommand(const Command& cmd);

    double lastKnownBPM = 120.0;
    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ARTEFACTAudioProcessor)
};
