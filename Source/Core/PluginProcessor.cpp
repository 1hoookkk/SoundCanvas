// Source/PluginProcessor.cpp
#include "PluginProcessor.h"
#include "GUI/PluginEditor.h"

//==============================================================================
// Constructor and Destructor

ARTEFACTAudioProcessor::ARTEFACTAudioProcessor()
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
    #if ! JucePlugin_IsSynth
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
    #endif
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
                     ),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Register as parameter listener for automatic parameter updates
    apvts.addParameterListener("masterGain", this);
    apvts.addParameterListener("paintActive", this);
    apvts.addParameterListener("processingMode", this);
}

ARTEFACTAudioProcessor::~ARTEFACTAudioProcessor()
{
    apvts.removeParameterListener("masterGain", this);
    apvts.removeParameterListener("paintActive", this);
    apvts.removeParameterListener("processingMode", this);
}

//==============================================================================
// Audio Processing Lifecycle

void ARTEFACTAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    // Prepare both processors
    forgeProcessor.prepareToPlay(sampleRate, samplesPerBlock);
    paintEngine.prepareToPlay(sampleRate, samplesPerBlock);
    
    // Set default active state based on current mode
    paintEngine.setActive(currentMode == ProcessingMode::Canvas || currentMode == ProcessingMode::Hybrid);
}

void ARTEFACTAudioProcessor::releaseResources()
{
    paintEngine.releaseResources();
    // Note: ForgeProcessor doesn't have releaseResources() method yet
}

//==============================================================================
// Parameter Management

juce::AudioProcessorValueTreeState::ParameterLayout ARTEFACTAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    // Master gain parameter
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "masterGain", "Master Gain", 0.0f, 2.0f, 0.7f));
    
    // Paint engine active parameter
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        "paintActive", "Paint Active", false));
    
    // Processing mode parameter
    parameters.push_back(std::make_unique<juce::AudioParameterChoice>(
        "processingMode", "Processing Mode", 
        juce::StringArray{"Forge", "Canvas", "Hybrid"}, 0));
    
    return { parameters.begin(), parameters.end() };
}

void ARTEFACTAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "masterGain")
    {
        paintEngine.setMasterGain(newValue);
    }
    else if (parameterID == "paintActive")
    {
        paintEngine.setActive(newValue > 0.5f);
    }
    else if (parameterID == "processingMode")
    {
        int modeIndex = static_cast<int>(newValue);
        currentMode = static_cast<ProcessingMode>(modeIndex);
        
        // Update paint engine active state based on mode
        bool shouldBeActive = (currentMode == ProcessingMode::Canvas || 
                              currentMode == ProcessingMode::Hybrid);
        paintEngine.setActive(shouldBeActive);
    }
}

//==============================================================================
// Editor Management

juce::AudioProcessorEditor* ARTEFACTAudioProcessor::createEditor()
{
    return new PluginEditor(*this);
}

//==============================================================================
// Bus Layout Support

bool ARTEFACTAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // Only mono/stereo supported
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

//==============================================================================
// State Management

void ARTEFACTAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Save plugin state
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ARTEFACTAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Restore plugin state
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(apvts.state.getType()))
        {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

//==============================================================================
// Command Queue Management

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
        
        // Route command based on type
        if (cmd.isForgeCommand())
        {
            processForgeCommand(cmd);
        }
        else if (cmd.isPaintCommand())
        {
            processPaintCommand(cmd);
        }
        
        abstractFifo.finishedRead(1);
    }
}

void ARTEFACTAudioProcessor::processForgeCommand(const Command& cmd)
{
    switch (cmd.getForgeCommandID())
    {
    case ForgeCommandID::StartPlayback:
        forgeProcessor.getVoice(cmd.intParam).start();
        break;
    case ForgeCommandID::StopPlayback:
        forgeProcessor.getVoice(cmd.intParam).stop();
        break;
    case ForgeCommandID::LoadSample:
        forgeProcessor.loadSampleIntoSlot(cmd.intParam, juce::File(cmd.stringParam));
        break;
    case ForgeCommandID::SetPitch:
        forgeProcessor.getVoice(cmd.intParam).setPitch(cmd.floatParam);
        break;
    case ForgeCommandID::SetSpeed:
        forgeProcessor.getVoice(cmd.intParam).setSpeed(cmd.floatParam);
        break;
    case ForgeCommandID::SetVolume:
        forgeProcessor.getVoice(cmd.intParam).setVolume(cmd.floatParam);
        break;
    case ForgeCommandID::SetDrive:
        forgeProcessor.getVoice(cmd.intParam).setDrive(cmd.floatParam);
        break;
    case ForgeCommandID::SetCrush:
        forgeProcessor.getVoice(cmd.intParam).setCrush(cmd.floatParam);
        break;
    case ForgeCommandID::SetSyncMode:
        forgeProcessor.getVoice(cmd.intParam).setSyncMode(cmd.boolParam);
        break;
    default:
        break;
    }
}

void ARTEFACTAudioProcessor::processPaintCommand(const Command& cmd)
{
    switch (cmd.getPaintCommandID())
    {
    case PaintCommandID::BeginStroke:
        paintEngine.beginStroke(PaintEngine::Point(cmd.x, cmd.y), cmd.pressure, cmd.color);
        break;
    case PaintCommandID::UpdateStroke:
        paintEngine.updateStroke(PaintEngine::Point(cmd.x, cmd.y), cmd.pressure);
        break;
    case PaintCommandID::EndStroke:
        paintEngine.endStroke();
        break;
    case PaintCommandID::ClearCanvas:
        paintEngine.clearCanvas();
        break;
    case PaintCommandID::SetPlayheadPosition:
        paintEngine.setPlayheadPosition(cmd.floatParam);
        break;
    case PaintCommandID::SetPaintActive:
        paintEngine.setActive(cmd.boolParam);
        break;
    case PaintCommandID::SetMasterGain:
        paintEngine.setMasterGain(cmd.floatParam);
        break;
    case PaintCommandID::SetFrequencyRange:
        paintEngine.setFrequencyRange(cmd.floatParam, static_cast<float>(cmd.doubleParam));
        break;
    case PaintCommandID::SetCanvasRegion:
        paintEngine.setCanvasRegion(cmd.x, cmd.y, cmd.floatParam, static_cast<float>(cmd.doubleParam));
        break;
    default:
        break;
    }
}

void ARTEFACTAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    
    // Process all pending commands
    for (int i = 0; i < fifoSize && abstractFifo.getNumReady() > 0; ++i)
        processNextCommand();

    // Update BPM if available from host
    if (auto playHead = getPlayHead())
    {
        if (auto positionInfo = playHead->getPosition())
        {
            if (positionInfo->getBpm().hasValue())
            {
                double hostBPM = *positionInfo->getBpm();
                if (std::abs(hostBPM - lastKnownBPM) > 0.1)
                {
                    lastKnownBPM = hostBPM;
                    forgeProcessor.setHostBPM(hostBPM);
                }
            }
        }
    }

    // Process audio based on current mode
    switch (currentMode)
    {
    case ProcessingMode::Canvas:
        // Canvas mode: Only PaintEngine
        paintEngine.processBlock(buffer);
        break;
        
    case ProcessingMode::Forge:
        // Forge mode: Only ForgeProcessor
        forgeProcessor.processBlock(buffer, midi);
        break;
        
    case ProcessingMode::Hybrid:
        // Hybrid mode: Mix both processors
        {
            juce::AudioBuffer<float> paintBuffer(buffer.getNumChannels(), buffer.getNumSamples());
            paintBuffer.clear();
            
            // Process paint engine into separate buffer
            paintEngine.processBlock(paintBuffer);
            
            // Process forge engine into main buffer
            forgeProcessor.processBlock(buffer, midi);
            
            // Mix the two signals (50/50 for now - could be parameterized)
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                buffer.addFrom(ch, 0, paintBuffer, ch, 0, buffer.getNumSamples(), 0.5f);
            }
        }
        break;
    }
}

//==============================================================================
// Plugin Factory

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ARTEFACTAudioProcessor();
}
