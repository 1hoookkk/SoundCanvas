#pragma once

#include <JuceHeader.h>
#include "Core/PaintEngine.h"
#include "Core/Commands.h"

/**
 * RetroCanvasComponent - Terminal-aesthetic audio painting canvas
 * 
 * Inspired by Cool Edit Pro, CDP, and TempleOS aesthetics
 * Features real-time audio painting with immediate feedback
 * 
 * Core Philosophy:
 * - Instant audio gratification
 * - Dense information display
 * - Terminal/CRT monitor aesthetic  
 * - Zero-compromise functionality
 */
class RetroCanvasComponent : public juce::Component,
                           public juce::Timer,
                           public juce::MouseListener
{
public:
    //==============================================================================
    // Retro Color Palette
    
    struct RetroColors
    {
        static const juce::Colour TERMINAL_BLACK;      // #221100 - Dark brown-black background
        static const juce::Colour TERMINAL_GREEN;      // #00FF00 - Matrix green for text
        static const juce::Colour TERMINAL_AMBER;      // #FFAA00 - Amber CRT glow
        static const juce::Colour TERMINAL_CYAN;       // #00FFFF - Cyan highlights
        static const juce::Colour TERMINAL_WHITE;      // #F0F0E0 - Off-white text
        static const juce::Colour TERMINAL_RED;        // #FF0000 - Error/warning red
        static const juce::Colour TERMINAL_BLUE;       // #0088FF - Info blue
        static const juce::Colour TERMINAL_MAGENTA;    // #FF00FF - Special magenta
        static const juce::Colour CANVAS_BLACK;        // #000000 - Pure black canvas
        static const juce::Colour PAINT_BRIGHT;        // #FFFF00 - Bright paint yellow
        static const juce::Colour GRID_DIM;           // #333333 - Dim grid lines
    };
    
    //==============================================================================
    // Brush Types
    
    enum class BrushType
    {
        SineBrush = 0,      // Pure sine wave painting
        HarmonicBrush,      // Harmonic series generator
        NoiseBrush,         // Textural noise painting
        SampleBrush,        // Paint with loaded samples
        GranularPen,        // Micro-grain placement
        CDPMorph           // Spectral morphing tool
    };
    
    //==============================================================================
    // Canvas State
    
    struct CanvasState
    {
        float zoomLevel = 1.0f;           // Canvas zoom factor
        float scrollX = 0.0f;             // Horizontal scroll position
        float scrollY = 0.0f;             // Vertical scroll position
        float minFreq = 80.0f;            // Minimum frequency (Hz)
        float maxFreq = 8000.0f;          // Maximum frequency (Hz)
        float timeRange = 8.0f;           // Time range in seconds
        bool showGrid = true;             // Display frequency grid
        bool showWaveform = true;         // Display waveform preview
        bool snapToGrid = false;          // Snap painting to grid
        BrushType currentBrush = BrushType::SineBrush;
    };
    
    //==============================================================================
    // Main Interface
    
    RetroCanvasComponent();
    ~RetroCanvasComponent() override;
    
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    
    // Timer for real-time updates
    void timerCallback() override;
    
    // Canvas control
    void setCanvasState(const CanvasState& newState);
    const CanvasState& getCanvasState() const { return canvasState; }
    void clearCanvas();
    void resetView();
    
    // Brush control
    void setBrushType(BrushType type);
    void setBrushSize(float size) { brushSize = size; }
    void setBrushPressure(float pressure) { brushPressure = juce::jlimit(0.0f, 1.0f, pressure); }
    void setBrushColor(juce::Colour color) { brushColor = color; }
    
    // Audio integration
    void setPaintEngine(PaintEngine* engine) { paintEngine = engine; }
    void setCommandTarget(std::function<bool(const Command&)> target) { commandTarget = target; }
    
    // Performance monitoring
    void setPerformanceInfo(float cpuLoad, int activeOscillators, float latency);
    
private:
    //==============================================================================
    // Canvas Geometry
    
    struct CanvasGeometry
    {
        juce::Rectangle<int> canvasArea;      // Main painting area
        juce::Rectangle<int> timeRuler;       // Time ruler at bottom
        juce::Rectangle<int> freqRuler;       // Frequency ruler on left
        juce::Rectangle<int> waveformArea;    // Waveform preview area
        juce::Rectangle<int> statusBar;       // Status information bar
        
        float pixelsPerSecond = 100.0f;      // Time scale
        float pixelsPerOctave = 80.0f;       // Frequency scale
    };
    
    //==============================================================================
    // Drawing Methods
    
    void drawTerminalBorder(juce::Graphics& g, juce::Rectangle<int> area, 
                           const juce::String& title, juce::Colour borderColor);
    void drawFrequencyGrid(juce::Graphics& g, const CanvasGeometry& geom);
    void drawTimeGrid(juce::Graphics& g, const CanvasGeometry& geom);
    void drawPaintedStrokes(juce::Graphics& g, const CanvasGeometry& geom);
    void drawPlayhead(juce::Graphics& g, const CanvasGeometry& geom);
    void drawWaveformPreview(juce::Graphics& g, const CanvasGeometry& geom);
    void drawStatusBar(juce::Graphics& g, const CanvasGeometry& geom);
    void drawBrushCursor(juce::Graphics& g);
    
    // Retro-style text rendering
    void drawTerminalText(juce::Graphics& g, const juce::String& text, 
                         int x, int y, juce::Colour color, bool bold = false);
    void drawAsciiArt(juce::Graphics& g, const juce::String& art, 
                     juce::Rectangle<int> area, juce::Colour color);
    
    // Visual effects
    void drawParticleEffect(juce::Graphics& g, juce::Point<float> position, 
                           float intensity, juce::Colour color);
    void drawScanlines(juce::Graphics& g, juce::Rectangle<int> area);
    
    //==============================================================================
    // Coordinate Conversion
    
    juce::Point<float> screenToCanvas(juce::Point<int> screenPoint) const;
    juce::Point<int> canvasToScreen(juce::Point<float> canvasPoint) const;
    float screenYToFrequency(int screenY, const CanvasGeometry& geom) const;
    int frequencyToScreenY(float frequency, const CanvasGeometry& geom) const;
    float screenXToTime(int screenX, const CanvasGeometry& geom) const;
    int timeToScreenX(float time, const CanvasGeometry& geom) const;
    
    CanvasGeometry calculateGeometry() const;
    
    //==============================================================================
    // Painting Logic
    
    void beginPaintStroke(juce::Point<float> canvasPoint, float pressure);
    void updatePaintStroke(juce::Point<float> canvasPoint, float pressure);
    void endPaintStroke();
    
    void sendPaintCommand(PaintCommandID commandID, juce::Point<float> canvasPoint, 
                         float pressure = 1.0f);
    
    // Visual feedback for painting
    void addParticleAt(juce::Point<float> position, juce::Colour color);
    void updateParticles();
    
    //==============================================================================
    // Font Management
    
    juce::Font createTerminalFont(float size) const;
    juce::Font createBoldTerminalFont(float size) const;
    
    //==============================================================================
    // Member Variables
    
    // Canvas state
    CanvasState canvasState;
    
    // Painting state
    bool isPainting = false;
    BrushType currentBrushType = BrushType::SineBrush;
    float brushSize = 2.0f;
    float brushPressure = 1.0f;
    juce::Colour brushColor = RetroColors::PAINT_BRIGHT;
    juce::Point<float> lastPaintPoint;
    
    // Audio integration
    PaintEngine* paintEngine = nullptr;
    std::function<bool(const Command&)> commandTarget;
    
    // Performance monitoring
    float currentCPULoad = 0.0f;
    int currentActiveOscillators = 0;
    float currentLatency = 0.0f;
    
    // Visual effects
    struct Particle
    {
        juce::Point<float> position;
        juce::Point<float> velocity;
        juce::Colour color;
        float life = 1.0f;
        float size = 2.0f;
    };
    std::vector<Particle> particles;
    
    // Animation
    float animationTime = 0.0f;
    juce::Point<int> mousePosition;
    bool showCursor = true;
    
    // Cached geometry
    mutable CanvasGeometry cachedGeometry;
    mutable bool geometryNeedsUpdate = true;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RetroCanvasComponent)
};