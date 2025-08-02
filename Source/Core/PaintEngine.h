#pragma once

#include <JuceHeader.h>
#include <vector>
#include <memory>
#include <atomic>
#include <unordered_map>

/**
 * Real-time audio painting engine for SoundCanvas
 * Converts brush strokes and canvas interactions into live audio synthesis
 * 
 * Core Philosophy:
 * - Sub-10ms latency from stroke to sound
 * - Support for multiple synthesis engines
 * - Infinite canvas with efficient sparse storage
 * - MetaSynth-inspired X=time, Y=pitch mapping
 */
class PaintEngine
{
public:
    //==============================================================================
    // Core Types
    
    struct Point
    {
        float x = 0.0f;        // Canvas X coordinate (time domain)
        float y = 0.0f;        // Canvas Y coordinate (frequency domain)
        
        Point() = default;
        Point(float x_, float y_) : x(x_), y(y_) {}
        
        bool operator==(const Point& other) const noexcept
        {
            return juce::approximatelyEqual(x, other.x) && 
                   juce::approximatelyEqual(y, other.y);
        }
    };
    
    struct AudioParams
    {
        float frequency = 440.0f;     // Hz - derived from Y position
        float amplitude = 0.0f;       // 0.0-1.0 - derived from brush pressure/brightness
        float pan = 0.5f;             // 0.0=left, 0.5=center, 1.0=right
        float time = 0.0f;            // Temporal position in canvas
        
        // Extended parameters for advanced synthesis
        float filterCutoff = 1.0f;    // 0.0-1.0
        float resonance = 0.0f;       // 0.0-1.0
        float modDepth = 0.0f;        // 0.0-1.0
        
        AudioParams() = default;
        AudioParams(float freq, float amp, float p, float t)
            : frequency(freq), amplitude(amp), pan(p), time(t) {}
    };
    
    struct StrokePoint
    {
        Point position;
        float pressure = 1.0f;        // 0.0-1.0 from input device
        float velocity = 0.0f;        // Derived from stroke speed
        juce::Colour color;           // RGBA color information
        juce::uint32 timestamp;       // When this point was created
        
        StrokePoint() = default;
        StrokePoint(Point pos, float press, juce::Colour col)
            : position(pos), pressure(press), color(col), 
              timestamp(juce::Time::getMillisecondCounter()) {}
    };
    
    // Forward declarations
    class Stroke;
    class CanvasRegion;
    class SynthEngine;
    
    //==============================================================================
    // Main Interface
    
    PaintEngine();
    ~PaintEngine();
    
    // Audio processing lifecycle
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processBlock(juce::AudioBuffer<float>& buffer);
    void releaseResources();
    
    // Stroke interaction API
    void beginStroke(Point position, float pressure = 1.0f, juce::Colour color = juce::Colours::white);
    void updateStroke(Point position, float pressure = 1.0f);
    void endStroke();
    
    // Canvas control
    void setPlayheadPosition(float normalisedPosition);
    void setCanvasRegion(float leftX, float rightX, float bottomY, float topY);
    void clearCanvas();
    void clearRegion(const juce::Rectangle<float>& region);
    
    // Audio parameters
    void setActive(bool shouldBeActive) { isActive.store(shouldBeActive); }
    bool getActive() const { return isActive.load(); }
    void setMasterGain(float gain);
    void setFrequencyRange(float minHz, float maxHz);
    void setUsePanning(bool shouldUsePanning) { usePanning.store(shouldUsePanning); }
    
    // Canvas mapping functions
    float canvasYToFrequency(float y) const;
    float frequencyToCanvasY(float frequency) const;
    float canvasXToTime(float x) const;
    float timeToCanvasX(float time) const;
    
    // Performance monitoring
    float getCurrentCPULoad() const { return cpuLoad.load(); }
    int getActiveOscillatorCount() const { return activeOscillators.load(); }
    
private:
    //==============================================================================
    // Internal Classes
    
    /**
     * Represents a single oscillator/partial in the synthesis
     */
    class Oscillator
    {
    public:
        Oscillator() = default;
        
        void setParameters(const AudioParams& params);
        void updatePhase(float sampleRate);
        float getSample() const;
        bool isActive() const { return amplitude > 0.0001f || targetAmplitude > 0.0001f; }
        
        // Smooth parameter changes to prevent clicks
        void smoothParameters(float smoothingFactor = 0.05f);
        
    private:
        float frequency = 440.0f;
        float amplitude = 0.0f;
        float targetAmplitude = 0.0f;
        float phase = 0.0f;
        float pan = 0.5f;
        float targetPan = 0.5f;
        
        // Phase increment is cached for performance
        float phaseIncrement = 0.0f;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Oscillator)
    };
    
    /**
     * Represents a painted stroke on the canvas
     */
    class Stroke
    {
    public:
        Stroke(juce::uint32 id);
        
        void addPoint(const StrokePoint& point);
        void finalize();
        
        bool isActive() const { return !isFinalized || hasActiveOscillators(); }
        void updateOscillators(float currentTime, std::vector<Oscillator>& oscillatorPool);
        
        const std::vector<StrokePoint>& getPoints() const { return points; }
        juce::uint32 getId() const { return strokeId; }
        
    private:
        juce::uint32 strokeId;
        std::vector<StrokePoint> points;
        bool isFinalized = false;
        
        // Cached bounds for optimization
        juce::Rectangle<float> bounds;
        void updateBounds();
        
        bool hasActiveOscillators() const;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Stroke)
    };
    
    /**
     * Sparse storage for canvas regions
     */
    class CanvasRegion
    {
    public:
        static constexpr int REGION_SIZE = 64;  // 64x64 pixel regions
        
        CanvasRegion(int regionX, int regionY);
        
        void addStroke(std::shared_ptr<Stroke> stroke);
        void removeStroke(juce::uint32 strokeId);
        void updateOscillators(float currentTime, std::vector<Oscillator>& oscillatorPool);
        
        bool isEmpty() const { return strokes.empty(); }
        int getRegionX() const { return regionX; }
        int getRegionY() const { return regionY; }
        
    private:
        int regionX, regionY;
        std::vector<std::shared_ptr<Stroke>> strokes;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CanvasRegion)
    };
    
    //==============================================================================
    // Member Variables
    
    // Audio processing state
    std::atomic<bool> isActive{ false };
    std::atomic<bool> usePanning{ true };
    std::atomic<float> cpuLoad{ 0.0f };
    std::atomic<int> activeOscillators{ 0 };
    
    double sampleRate = 44100.0;
    int samplesPerBlock = 512;
    
    // Canvas state
    float playheadPosition = 0.0f;      // 0.0-1.0 normalized position
    float canvasLeft = -100.0f;         // Canvas bounds in arbitrary units
    float canvasRight = 100.0f;
    float canvasBottom = -50.0f;
    float canvasTop = 50.0f;
    
    // Frequency mapping
    float minFrequency = 20.0f;
    float maxFrequency = 20000.0f;
    bool useLogFrequencyScale = true;
    
    // Oscillator pool for performance
    static constexpr int MAX_OSCILLATORS = 1024;
    std::vector<Oscillator> oscillatorPool;
    std::atomic<int> oscillatorPoolIndex{ 0 };
    
    // Stroke management
    std::unique_ptr<Stroke> currentStroke;
    juce::uint32 nextStrokeId = 1;
    
    // Sparse canvas storage
    std::unordered_map<juce::int64, std::unique_ptr<CanvasRegion>> canvasRegions;
    
    // Audio processing
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> masterGain;
    
    // Performance monitoring
    juce::Time::MillisecondCounter lastProcessTime = 0;
    juce::CriticalSection oscillatorLock;
    
    //==============================================================================
    // Private Methods
    
    void updateCanvasOscillators();
    juce::int64 getRegionKey(int regionX, int regionY) const;
    CanvasRegion* getOrCreateRegion(float canvasX, float canvasY);
    void cullInactiveRegions();
    
    // Audio parameter conversion
    AudioParams strokePointToAudioParams(const StrokePoint& point) const;
    
    // Performance optimization
    void updateCPULoad();
    void optimizeOscillatorPool();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PaintEngine)
};