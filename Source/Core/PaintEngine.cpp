#include "PaintEngine.h"
#include <algorithm>
#include <cmath>

//==============================================================================
// PaintEngine Implementation

PaintEngine::PaintEngine()
{
    // Initialize oscillator pool
    oscillatorPool.resize(MAX_OSCILLATORS);
    
    // Set default canvas bounds for typical musical range
    setFrequencyRange(20.0f, 20000.0f);
    setCanvasRegion(-100.0f, 100.0f, -50.0f, 50.0f);
}

PaintEngine::~PaintEngine()
{
    releaseResources();
}

void PaintEngine::prepareToPlay(double sr, int samplesPerBlock_)
{
    sampleRate = sr;
    samplesPerBlock = samplesPerBlock_;
    
    // Initialize smoothed values
    masterGain.reset(sampleRate, 0.01); // 10ms smoothing
    masterGain.setCurrentAndTargetValue(0.7f);
    
    // Reset oscillator pool
    for (auto& osc : oscillatorPool)
    {
        osc = Oscillator{}; // Reset to default state
    }
    
    oscillatorPoolIndex.store(0);
    activeOscillators.store(0);
    
    DBG("PaintEngine prepared: " << sampleRate << "Hz, " << samplesPerBlock_ << " samples");
}

void PaintEngine::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (!isActive.load())
    {
        buffer.clear();
        return;
    }
    
    const auto startTime = juce::Time::getMillisecondCounterHiRes();
    
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    // Clear buffer
    buffer.clear();
    
    // Get buffer pointers for efficient access
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;
    
    // Update canvas oscillators based on current playhead position
    updateCanvasOscillators();
    
    // Process audio sample by sample
    const juce::ScopedLock lock(oscillatorLock);
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        float leftSample = 0.0f;
        float rightSample = 0.0f;
        
        int activeOscCount = 0;
        
        // Process active oscillators
        for (auto& osc : oscillatorPool)
        {
            if (osc.isActive())
            {
                osc.smoothParameters();
                const float oscSample = osc.getSample();
                
                if (usePanning.load() && rightChannel != nullptr)
                {
                    // Stereo panning
                    const float panValue = 0.5f; // TODO: Get actual pan from oscillator
                    leftSample += oscSample * (1.0f - panValue);
                    rightSample += oscSample * panValue;
                }
                else
                {
                    // Mono or no panning
                    leftSample += oscSample;
                }
                
                osc.updatePhase(static_cast<float>(sampleRate));
                ++activeOscCount;
            }
        }
        
        // Apply master gain
        const float currentGain = masterGain.getNextValue();
        leftChannel[sample] = leftSample * currentGain;
        
        if (rightChannel != nullptr)
        {
            rightChannel[sample] = usePanning.load() ? (rightSample * currentGain) : leftChannel[sample];
        }
        
        // Update active oscillator count periodically
        if (sample == 0)
        {
            activeOscillators.store(activeOscCount);
        }
    }
    
    // Update performance metrics
    const auto endTime = juce::Time::getMillisecondCounterHiRes();
    const float processingTime = static_cast<float>(endTime - startTime);
    const float blockDuration = static_cast<float>(numSamples) / static_cast<float>(sampleRate) * 1000.0f;
    cpuLoad.store(processingTime / blockDuration);
    
    // Periodically optimize oscillator pool
    static int blockCounter = 0;
    if (++blockCounter % 100 == 0) // Every ~2 seconds at 44.1kHz/512 samples
    {
        optimizeOscillatorPool();
    }
}

void PaintEngine::releaseResources()
{
    const juce::ScopedLock lock(oscillatorLock);
    
    currentStroke.reset();
    canvasRegions.clear();
    
    for (auto& osc : oscillatorPool)
    {
        osc = Oscillator{};
    }
    
    oscillatorPoolIndex.store(0);
    activeOscillators.store(0);
}

//==============================================================================
// Stroke Interaction API

void PaintEngine::beginStroke(Point position, float pressure, juce::Colour color)
{
    if (currentStroke != nullptr)
    {
        // End previous stroke if one was active
        endStroke();
    }
    
    currentStroke = std::make_unique<Stroke>(nextStrokeId++);
    
    StrokePoint point(position, pressure, color);
    currentStroke->addPoint(point);
    
    DBG("Stroke started at (" << position.x << ", " << position.y << ") pressure=" << pressure);
}

void PaintEngine::updateStroke(Point position, float pressure)
{
    if (currentStroke == nullptr)
    {
        // Auto-start stroke if none active
        beginStroke(position, pressure);
        return;
    }
    
    StrokePoint point(position, pressure, juce::Colours::white);
    currentStroke->addPoint(point);
    
    // Update oscillators in real-time for immediate feedback
    updateCanvasOscillators();
}

void PaintEngine::endStroke()
{
    if (currentStroke == nullptr)
        return;
    
    currentStroke->finalize();
    
    // Add stroke to appropriate canvas regions
    const auto& points = currentStroke->getPoints();
    if (!points.empty())
    {
        for (const auto& point : points)
        {
            auto* region = getOrCreateRegion(point.position.x, point.position.y);
            if (region != nullptr)
            {
                region->addStroke(std::move(currentStroke));
                break; // Only add to first region for now
            }
        }
    }
    
    currentStroke.reset();
    
    DBG("Stroke ended and added to canvas");
}

//==============================================================================
// Canvas Control

void PaintEngine::setPlayheadPosition(float normalisedPosition)
{
    playheadPosition = juce::jlimit(0.0f, 1.0f, normalisedPosition);
}

void PaintEngine::setCanvasRegion(float leftX, float rightX, float bottomY, float topY)
{
    canvasLeft = leftX;
    canvasRight = rightX;
    canvasBottom = bottomY;
    canvasTop = topY;
}

void PaintEngine::clearCanvas()
{
    const juce::ScopedLock lock(oscillatorLock);
    
    currentStroke.reset();
    canvasRegions.clear();
    
    // Reset all oscillators
    for (auto& osc : oscillatorPool)
    {
        osc = Oscillator{};
    }
    
    oscillatorPoolIndex.store(0);
    activeOscillators.store(0);
    
    DBG("Canvas cleared");
}

void PaintEngine::clearRegion(const juce::Rectangle<float>& region)
{
    // TODO: Implement selective region clearing
    juce::ignoreUnused(region);
}

void PaintEngine::setMasterGain(float gain)
{
    masterGain.setTargetValue(juce::jlimit(0.0f, 2.0f, gain));
}

void PaintEngine::setFrequencyRange(float minHz, float maxHz)
{
    minFrequency = juce::jlimit(1.0f, 20000.0f, minHz);
    maxFrequency = juce::jlimit(minFrequency + 1.0f, 22000.0f, maxHz);
}

//==============================================================================
// Canvas Mapping Functions

float PaintEngine::canvasYToFrequency(float y) const
{
    // Normalize Y to 0-1 range
    const float normalizedY = (y - canvasBottom) / (canvasTop - canvasBottom);
    const float clampedY = juce::jlimit(0.0f, 1.0f, normalizedY);
    
    if (useLogFrequencyScale)
    {
        // Logarithmic frequency mapping (more musical)
        const float logMin = std::log(minFrequency);
        const float logMax = std::log(maxFrequency);
        return std::exp(logMin + clampedY * (logMax - logMin));
    }
    else
    {
        // Linear frequency mapping
        return minFrequency + clampedY * (maxFrequency - minFrequency);
    }
}

float PaintEngine::frequencyToCanvasY(float frequency) const
{
    const float clampedFreq = juce::jlimit(minFrequency, maxFrequency, frequency);
    
    float normalizedY;
    if (useLogFrequencyScale)
    {
        const float logMin = std::log(minFrequency);
        const float logMax = std::log(maxFrequency);
        const float logFreq = std::log(clampedFreq);
        normalizedY = (logFreq - logMin) / (logMax - logMin);
    }
    else
    {
        normalizedY = (clampedFreq - minFrequency) / (maxFrequency - minFrequency);
    }
    
    return canvasBottom + normalizedY * (canvasTop - canvasBottom);
}

float PaintEngine::canvasXToTime(float x) const
{
    // Simple linear mapping for now
    const float normalizedX = (x - canvasLeft) / (canvasRight - canvasLeft);
    return juce::jlimit(0.0f, 1.0f, normalizedX);
}

float PaintEngine::timeToCanvasX(float time) const
{
    const float clampedTime = juce::jlimit(0.0f, 1.0f, time);
    return canvasLeft + clampedTime * (canvasRight - canvasLeft);
}

//==============================================================================
// Private Methods

void PaintEngine::updateCanvasOscillators()
{
    // Update oscillators based on current playhead position and active strokes
    const float currentTime = canvasXToTime(playheadPosition * (canvasRight - canvasLeft) + canvasLeft);
    
    // Process current stroke if active
    if (currentStroke != nullptr)
    {
        const juce::ScopedLock lock(oscillatorLock);
        currentStroke->updateOscillators(currentTime, oscillatorPool);
    }
    
    // Process stored canvas regions
    for (auto& [key, region] : canvasRegions)
    {
        if (region != nullptr && !region->isEmpty())
        {
            const juce::ScopedLock lock(oscillatorLock);
            region->updateOscillators(currentTime, oscillatorPool);
        }
    }
}

juce::int64 PaintEngine::getRegionKey(int regionX, int regionY) const
{
    return (static_cast<juce::int64>(regionX) << 32) | static_cast<juce::int64>(regionY);
}

PaintEngine::CanvasRegion* PaintEngine::getOrCreateRegion(float canvasX, float canvasY)
{
    const int regionX = static_cast<int>(std::floor(canvasX / CanvasRegion::REGION_SIZE));
    const int regionY = static_cast<int>(std::floor(canvasY / CanvasRegion::REGION_SIZE));
    const juce::int64 key = getRegionKey(regionX, regionY);
    
    auto it = canvasRegions.find(key);
    if (it != canvasRegions.end())
    {
        return it->second.get();
    }
    
    // Create new region
    auto newRegion = std::make_unique<CanvasRegion>(regionX, regionY);
    auto* regionPtr = newRegion.get();
    canvasRegions[key] = std::move(newRegion);
    
    return regionPtr;
}

void PaintEngine::cullInactiveRegions()
{
    auto it = canvasRegions.begin();
    while (it != canvasRegions.end())
    {
        if (it->second->isEmpty())
        {
            it = canvasRegions.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

AudioParams PaintEngine::strokePointToAudioParams(const StrokePoint& point) const
{
    AudioParams params;
    
    params.frequency = canvasYToFrequency(point.position.y);
    params.amplitude = point.pressure; // Direct mapping for now
    params.time = canvasXToTime(point.position.x);
    
    // Extract pan from color hue
    if (point.color != juce::Colours::transparentBlack)
    {
        params.pan = point.color.getHue();
    }
    else
    {
        params.pan = 0.5f; // Center
    }
    
    return params;
}

void PaintEngine::updateCPULoad()
{
    // CPU load is updated in processBlock()
}

void PaintEngine::optimizeOscillatorPool()
{
    // Reset inactive oscillators to improve cache performance
    int compactIndex = 0;
    for (int i = 0; i < MAX_OSCILLATORS; ++i)
    {
        if (oscillatorPool[i].isActive())
        {
            if (compactIndex != i)
            {
                oscillatorPool[compactIndex] = std::move(oscillatorPool[i]);
                oscillatorPool[i] = Oscillator{};
            }
            ++compactIndex;
        }
    }
    
    oscillatorPoolIndex.store(compactIndex);
}

//==============================================================================
// Oscillator Implementation

void PaintEngine::Oscillator::setParameters(const AudioParams& params)
{
    frequency = params.frequency;
    targetAmplitude = juce::jlimit(0.0f, 1.0f, params.amplitude);
    targetPan = juce::jlimit(0.0f, 1.0f, params.pan);
}

void PaintEngine::Oscillator::updatePhase(float sampleRate)
{
    phaseIncrement = frequency / sampleRate;
    phase += phaseIncrement;
    
    // Wrap phase to [0, 1)
    if (phase >= 1.0f)
        phase -= std::floor(phase);
}

float PaintEngine::Oscillator::getSample() const
{
    return std::sin(phase * juce::MathConstants<float>::twoPi) * amplitude;
}

void PaintEngine::Oscillator::smoothParameters(float smoothingFactor)
{
    amplitude += (targetAmplitude - amplitude) * smoothingFactor;
    pan += (targetPan - pan) * smoothingFactor;
}

//==============================================================================
// Stroke Implementation

PaintEngine::Stroke::Stroke(juce::uint32 id) 
    : strokeId(id)
{
    points.reserve(256); // Reserve space for typical stroke
}

void PaintEngine::Stroke::addPoint(const StrokePoint& point)
{
    points.push_back(point);
    updateBounds();
}

void PaintEngine::Stroke::finalize()
{
    isFinalized = true;
    updateBounds();
}

void PaintEngine::Stroke::updateOscillators(float currentTime, std::vector<Oscillator>& oscillatorPool)
{
    // Simple implementation: activate oscillators for visible points
    juce::ignoreUnused(currentTime, oscillatorPool);
    
    // TODO: Implement sophisticated stroke-to-oscillator mapping
}

void PaintEngine::Stroke::updateBounds()
{
    if (points.empty())
    {
        bounds = juce::Rectangle<float>();
        return;
    }
    
    float minX = points[0].position.x;
    float maxX = minX;
    float minY = points[0].position.y;
    float maxY = minY;
    
    for (const auto& point : points)
    {
        minX = std::min(minX, point.position.x);
        maxX = std::max(maxX, point.position.x);
        minY = std::min(minY, point.position.y);
        maxY = std::max(maxY, point.position.y);
    }
    
    bounds = juce::Rectangle<float>(minX, minY, maxX - minX, maxY - minY);
}

bool PaintEngine::Stroke::hasActiveOscillators() const
{
    // TODO: Implement proper oscillator tracking
    return !points.empty() && !isFinalized;
}

//==============================================================================
// CanvasRegion Implementation

PaintEngine::CanvasRegion::CanvasRegion(int regionX_, int regionY_)
    : regionX(regionX_), regionY(regionY_)
{
    strokes.reserve(16); // Reserve space for typical region
}

void PaintEngine::CanvasRegion::addStroke(std::shared_ptr<Stroke> stroke)
{
    if (stroke != nullptr)
    {
        strokes.push_back(std::move(stroke));
    }
}

void PaintEngine::CanvasRegion::removeStroke(juce::uint32 strokeId)
{
    strokes.erase(
        std::remove_if(strokes.begin(), strokes.end(),
            [strokeId](const std::shared_ptr<Stroke>& stroke) {
                return stroke->getId() == strokeId;
            }),
        strokes.end());
}

void PaintEngine::CanvasRegion::updateOscillators(float currentTime, std::vector<Oscillator>& oscillatorPool)
{
    for (auto& stroke : strokes)
    {
        if (stroke != nullptr && stroke->isActive())
        {
            stroke->updateOscillators(currentTime, oscillatorPool);
        }
    }
}