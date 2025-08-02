#include "RetroCanvasComponent.h"
#include <cmath>

//==============================================================================
// Retro Color Definitions

const juce::Colour RetroCanvasComponent::RetroColors::TERMINAL_BLACK    = juce::Colour(0x22, 0x11, 0x00);
const juce::Colour RetroCanvasComponent::RetroColors::TERMINAL_GREEN    = juce::Colour(0x00, 0xFF, 0x00);
const juce::Colour RetroCanvasComponent::RetroColors::TERMINAL_AMBER    = juce::Colour(0xFF, 0xAA, 0x00);
const juce::Colour RetroCanvasComponent::RetroColors::TERMINAL_CYAN     = juce::Colour(0x00, 0xFF, 0xFF);
const juce::Colour RetroCanvasComponent::RetroColors::TERMINAL_WHITE    = juce::Colour(0xF0, 0xF0, 0xE0);
const juce::Colour RetroCanvasComponent::RetroColors::TERMINAL_RED      = juce::Colour(0xFF, 0x00, 0x00);
const juce::Colour RetroCanvasComponent::RetroColors::TERMINAL_BLUE     = juce::Colour(0x00, 0x88, 0xFF);
const juce::Colour RetroCanvasComponent::RetroColors::TERMINAL_MAGENTA  = juce::Colour(0xFF, 0x00, 0xFF);
const juce::Colour RetroCanvasComponent::RetroColors::CANVAS_BLACK      = juce::Colour(0x00, 0x00, 0x00);
const juce::Colour RetroCanvasComponent::RetroColors::PAINT_BRIGHT      = juce::Colour(0xFF, 0xFF, 0x00);
const juce::Colour RetroCanvasComponent::RetroColors::GRID_DIM          = juce::Colour(0x33, 0x33, 0x33);

//==============================================================================
// Constructor and Destructor

RetroCanvasComponent::RetroCanvasComponent()
{
    setMouseCursor(juce::MouseCursor::CrosshairCursor);
    setWantsKeyboardFocus(true);
    
    // Start timer for animation updates (60 FPS)
    startTimerHz(60);
    
    // Initialize canvas state with sensible defaults
    canvasState.minFreq = 80.0f;      // Musical bass range
    canvasState.maxFreq = 8000.0f;    // Upper treble
    canvasState.timeRange = 8.0f;     // 8 seconds visible
    canvasState.zoomLevel = 1.0f;
    canvasState.showGrid = true;
    canvasState.showWaveform = true;
    
    DBG("RetroCanvasComponent initialized - ready for audio painting!");
}

RetroCanvasComponent::~RetroCanvasComponent()
{
    stopTimer();
}

//==============================================================================
// Component Overrides

void RetroCanvasComponent::paint(juce::Graphics& g)
{
    // Calculate layout geometry
    const auto geom = calculateGeometry();
    
    // Fill background with terminal black
    g.fillAll(RetroColors::TERMINAL_BLACK);
    
    // Draw main canvas area
    g.setColour(RetroColors::CANVAS_BLACK);
    g.fillRect(geom.canvasArea);
    
    // Draw terminal-style border around canvas
    drawTerminalBorder(g, geom.canvasArea, "SPECTRAL PAINT CANVAS", RetroColors::TERMINAL_GREEN);
    
    // Draw frequency and time grids
    if (canvasState.showGrid)
    {
        drawFrequencyGrid(g, geom);
        drawTimeGrid(g, geom);
    }
    
    // Draw painted strokes (audio visualization)
    drawPaintedStrokes(g, geom);
    
    // Draw playhead position
    drawPlayhead(g, geom);
    
    // Draw waveform preview
    if (canvasState.showWaveform)
    {
        drawWaveformPreview(g, geom);
    }
    
    // Draw status bar with performance info
    drawStatusBar(g, geom);
    
    // Draw brush cursor
    if (showCursor && isMouseOver())
    {
        drawBrushCursor(g);
    }
    
    // Draw CRT-style scanlines for authentic retro feel
    drawScanlines(g, getLocalBounds());
    
    // Draw particles for visual feedback
    for (const auto& particle : particles)
    {
        drawParticleEffect(g, particle.position, particle.life, particle.color);
    }
}

void RetroCanvasComponent::resized()
{
    geometryNeedsUpdate = true;
}

void RetroCanvasComponent::mouseDown(const juce::MouseEvent& e)
{
    const auto geom = calculateGeometry();
    
    if (!geom.canvasArea.contains(e.getPosition()))
        return;
        
    const auto canvasPoint = screenToCanvas(e.getPosition());
    const float pressure = e.mods.isLeftButtonDown() ? 1.0f : 0.5f;
    
    beginPaintStroke(canvasPoint, pressure);
    
    // Add particle effect at click point
    addParticleAt(e.getPosition().toFloat(), brushColor);
    
    DBG("Paint stroke started at (" << canvasPoint.x << ", " << canvasPoint.y << ")");
}

void RetroCanvasComponent::mouseDrag(const juce::MouseEvent& e)
{
    mousePosition = e.getPosition();
    
    const auto geom = calculateGeometry();
    
    if (!isPainting || !geom.canvasArea.contains(e.getPosition()))
        return;
        
    const auto canvasPoint = screenToCanvas(e.getPosition());
    const float pressure = e.mods.isLeftButtonDown() ? 1.0f : 0.5f;
    
    updatePaintStroke(canvasPoint, pressure);
    
    // Add continuous particle trail
    if (juce::Random::getSystemRandom().nextFloat() < 0.3f)
    {
        addParticleAt(e.getPosition().toFloat(), brushColor);
    }
}

void RetroCanvasComponent::mouseUp(const juce::MouseEvent& e)
{
    juce::ignoreUnused(e);
    
    if (isPainting)
    {
        endPaintStroke();
        DBG("Paint stroke completed");
    }
}

void RetroCanvasComponent::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    // Zoom with mouse wheel
    if (e.mods.isCtrlDown())
    {
        float zoomFactor = 1.0f + (wheel.deltaY * 0.1f);
        canvasState.zoomLevel = juce::jlimit(0.1f, 10.0f, canvasState.zoomLevel * zoomFactor);
        geometryNeedsUpdate = true;
        repaint();
    }
    // Scroll vertically (frequency)
    else if (e.mods.isShiftDown())
    {
        canvasState.scrollY += wheel.deltaY * 50.0f;
        repaint();
    }
    // Scroll horizontally (time)
    else
    {
        canvasState.scrollX += wheel.deltaX * 50.0f;
        repaint();
    }
}

void RetroCanvasComponent::timerCallback()
{
    // Update animation time
    animationTime += 1.0f / 60.0f;
    
    // Update particles
    updateParticles();
    
    // Cursor blinking effect
    showCursor = std::sin(animationTime * 4.0f) > 0.0f;
    
    // Repaint for smooth animation
    repaint();
}

//==============================================================================
// Drawing Methods

void RetroCanvasComponent::drawTerminalBorder(juce::Graphics& g, juce::Rectangle<int> area, 
                                            const juce::String& title, juce::Colour borderColor)
{
    // Draw double-line ASCII border
    g.setColour(borderColor);
    
    // Outer border
    g.drawRect(area, 2);
    
    // Inner border
    area = area.reduced(3);
    g.drawRect(area, 1);
    
    // Title bar
    if (title.isNotEmpty())
    {
        const int titleHeight = 20;
        auto titleArea = area.removeFromTop(titleHeight);
        
        g.setColour(borderColor.withAlpha(0.3f));
        g.fillRect(titleArea);
        
        g.setColour(borderColor);
        g.drawRect(titleArea, 1);
        
        g.setFont(createBoldTerminalFont(12.0f));
        g.drawText("★ " + title + " ★", titleArea, juce::Justification::centred);
    }
}

void RetroCanvasComponent::drawFrequencyGrid(juce::Graphics& g, const CanvasGeometry& geom)
{
    g.setColour(RetroColors::GRID_DIM);
    g.setFont(createTerminalFont(10.0f));
    
    // Draw frequency grid lines and labels
    const float logMinFreq = std::log2(canvasState.minFreq);
    const float logMaxFreq = std::log2(canvasState.maxFreq);
    
    // Draw octave lines
    for (float octave = std::ceil(logMinFreq); octave <= logMaxFreq; octave += 1.0f)
    {
        const float freq = std::pow(2.0f, octave);
        const int y = frequencyToScreenY(freq, geom);
        
        if (y >= geom.canvasArea.getY() && y <= geom.canvasArea.getBottom())
        {
            // Draw grid line
            g.drawLine(geom.canvasArea.getX(), y, geom.canvasArea.getRight(), y, 1.0f);
            
            // Draw frequency label
            juce::String freqLabel;
            if (freq >= 1000.0f)
                freqLabel = juce::String(freq / 1000.0f, 1) + "k";
            else
                freqLabel = juce::String((int)freq);
                
            g.drawText(freqLabel + "Hz", 
                      geom.freqRuler.getX(), y - 8, 
                      geom.freqRuler.getWidth() - 5, 16, 
                      juce::Justification::centredRight);
        }
    }
    
    // Draw sub-octave lines (dimmer)
    g.setColour(RetroColors::GRID_DIM.withAlpha(0.3f));
    for (float octave = logMinFreq; octave <= logMaxFreq; octave += 0.5f)
    {
        const float freq = std::pow(2.0f, octave);
        const int y = frequencyToScreenY(freq, geom);
        
        if (y >= geom.canvasArea.getY() && y <= geom.canvasArea.getBottom())
        {
            g.drawLine(geom.canvasArea.getX(), y, geom.canvasArea.getRight(), y, 1.0f);
        }
    }
}

void RetroCanvasComponent::drawTimeGrid(juce::Graphics& g, const CanvasGeometry& geom)
{
    g.setColour(RetroColors::GRID_DIM);
    g.setFont(createTerminalFont(10.0f));
    
    // Draw time grid lines and labels
    const float timeStep = 1.0f; // 1 second intervals
    
    for (float time = 0.0f; time <= canvasState.timeRange; time += timeStep)
    {
        const int x = timeToScreenX(time, geom);
        
        if (x >= geom.canvasArea.getX() && x <= geom.canvasArea.getRight())
        {
            // Draw grid line
            g.drawLine(x, geom.canvasArea.getY(), x, geom.canvasArea.getBottom(), 1.0f);
            
            // Draw time label
            juce::String timeLabel = juce::String(time, 1) + "s";
            g.drawText(timeLabel,
                      x - 20, geom.timeRuler.getY(),
                      40, geom.timeRuler.getHeight(),
                      juce::Justification::centred);
        }
    }
    
    // Draw sub-second lines (dimmer)
    g.setColour(RetroColors::GRID_DIM.withAlpha(0.3f));
    for (float time = 0.0f; time <= canvasState.timeRange; time += 0.25f)
    {
        const int x = timeToScreenX(time, geom);
        
        if (x >= geom.canvasArea.getX() && x <= geom.canvasArea.getRight())
        {
            g.drawLine(x, geom.canvasArea.getY(), x, geom.canvasArea.getBottom(), 1.0f);
        }
    }
}

void RetroCanvasComponent::drawPaintedStrokes(juce::Graphics& g, const CanvasGeometry& geom)
{
    // TODO: Get painted strokes from PaintEngine and visualize them
    // For now, draw a placeholder pattern
    
    g.setColour(RetroColors::PAINT_BRIGHT.withAlpha(0.7f));
    
    // Draw some example painted areas as bright blocks
    for (int i = 0; i < 10; ++i)
    {
        const float time = i * 0.8f;
        const float freq = 200.0f + i * 100.0f;
        const int x = timeToScreenX(time, geom);
        const int y = frequencyToScreenY(freq, geom);
        const int width = (int)(geom.pixelsPerSecond * 0.3f);
        const int height = 8;
        
        if (x >= geom.canvasArea.getX() && x <= geom.canvasArea.getRight() &&
            y >= geom.canvasArea.getY() && y <= geom.canvasArea.getBottom())
        {
            g.fillRect(x, y - height/2, width, height);
            
            // Add glow effect
            g.setColour(RetroColors::PAINT_BRIGHT.withAlpha(0.3f));
            g.fillRect(x - 1, y - height/2 - 1, width + 2, height + 2);
            g.setColour(RetroColors::PAINT_BRIGHT.withAlpha(0.7f));
        }
    }
}

void RetroCanvasComponent::drawPlayhead(juce::Graphics& g, const CanvasGeometry& geom)
{
    // TODO: Get actual playhead position from PaintEngine
    const float playheadTime = std::fmod(animationTime, canvasState.timeRange);
    const int playheadX = timeToScreenX(playheadTime, geom);
    
    if (playheadX >= geom.canvasArea.getX() && playheadX <= geom.canvasArea.getRight())
    {
        // Draw playhead line with glow effect
        g.setColour(RetroColors::TERMINAL_CYAN.withAlpha(0.8f));
        g.drawLine(playheadX, geom.canvasArea.getY(), 
                   playheadX, geom.canvasArea.getBottom(), 2.0f);
        
        // Add glow
        g.setColour(RetroColors::TERMINAL_CYAN.withAlpha(0.3f));
        g.drawLine(playheadX - 1, geom.canvasArea.getY(), 
                   playheadX - 1, geom.canvasArea.getBottom(), 4.0f);
        g.drawLine(playheadX + 1, geom.canvasArea.getY(), 
                   playheadX + 1, geom.canvasArea.getBottom(), 4.0f);
        
        // Draw playhead triangle at top
        juce::Path triangle;
        triangle.addTriangle(playheadX - 5, geom.canvasArea.getY() - 5,
                           playheadX + 5, geom.canvasArea.getY() - 5,
                           playheadX, geom.canvasArea.getY());
        g.setColour(RetroColors::TERMINAL_CYAN);
        g.fillPath(triangle);
    }
}

void RetroCanvasComponent::drawWaveformPreview(juce::Graphics& g, const CanvasGeometry& geom)
{
    // TODO: Get actual waveform data from PaintEngine
    // For now, draw a placeholder waveform
    
    g.setColour(RetroColors::TERMINAL_GREEN.withAlpha(0.6f));
    
    const int centerY = geom.waveformArea.getCentreY();
    const int amplitude = geom.waveformArea.getHeight() / 4;
    
    juce::Path waveform;
    bool firstPoint = true;
    
    for (int x = geom.waveformArea.getX(); x < geom.waveformArea.getRight(); x += 2)
    {
        const float time = screenXToTime(x, geom);
        const float sample = std::sin(time * 440.0f * 2.0f * juce::MathConstants<float>::pi) * 
                           std::exp(-time * 2.0f); // Decay envelope
        const int y = centerY + (int)(sample * amplitude);
        
        if (firstPoint)
        {
            waveform.startNewSubPath(x, y);
            firstPoint = false;
        }
        else
        {
            waveform.lineTo(x, y);
        }
    }
    
    g.strokePath(waveform, juce::PathStrokeType(1.0f));
    
    // Draw waveform area border
    g.setColour(RetroColors::TERMINAL_GREEN);
    g.drawRect(geom.waveformArea, 1);
    
    // Add "WAVEFORM" label
    g.setFont(createTerminalFont(8.0f));
    g.drawText("WAVEFORM", 
              geom.waveformArea.getX() + 2, geom.waveformArea.getY() + 2,
              60, 10, juce::Justification::left);
}

void RetroCanvasComponent::drawStatusBar(juce::Graphics& g, const CanvasGeometry& geom)
{
    g.setColour(RetroColors::TERMINAL_BLACK.brighter(0.1f));
    g.fillRect(geom.statusBar);
    
    g.setColour(RetroColors::TERMINAL_GREEN);
    g.drawRect(geom.statusBar, 1);
    
    g.setFont(createTerminalFont(10.0f));
    g.setColour(RetroColors::TERMINAL_WHITE);
    
    // Status text with performance info
    juce::String statusText = 
        juce::String::formatted("STATUS: %s | OSC: %04d/1024 | CPU: %02d%% | LAT: %03dms | ZOOM: %.1fx",
                               isPainting ? "PAINTING" : "READY",
                               currentActiveOscillators,
                               (int)(currentCPULoad * 100.0f),
                               (int)(currentLatency * 1000.0f),
                               canvasState.zoomLevel);
    
    g.drawText(statusText, 
              geom.statusBar.reduced(5, 2), 
              juce::Justification::centredLeft);
    
    // Draw brush type indicator
    juce::String brushText = "BRUSH: ";
    switch (currentBrushType)
    {
        case BrushType::SineBrush:     brushText += "SINE"; break;
        case BrushType::HarmonicBrush: brushText += "HARMONIC"; break;
        case BrushType::NoiseBrush:    brushText += "NOISE"; break;
        case BrushType::SampleBrush:   brushText += "SAMPLE"; break;
        case BrushType::GranularPen:   brushText += "GRANULAR"; break;
        case BrushType::CDPMorph:      brushText += "CDP-MORPH"; break;
    }
    
    g.setColour(RetroColors::TERMINAL_AMBER);
    g.drawText(brushText, 
              geom.statusBar.getRight() - 120, geom.statusBar.getY() + 2,
              115, geom.statusBar.getHeight() - 4, 
              juce::Justification::centredRight);
}

void RetroCanvasComponent::drawBrushCursor(juce::Graphics& g)
{
    const auto geom = calculateGeometry();
    
    if (!geom.canvasArea.contains(mousePosition))
        return;
    
    // Draw crosshair cursor
    g.setColour(brushColor.withAlpha(0.8f));
    
    const int size = (int)(brushSize * 10.0f);
    const int x = mousePosition.x;
    const int y = mousePosition.y;
    
    // Crosshair lines
    g.drawLine(x - size, y, x + size, y, 1.0f);
    g.drawLine(x, y - size, x, y + size, 1.0f);
    
    // Brush size circle
    g.drawEllipse(x - size/2, y - size/2, size, size, 1.0f);
    
    // Add frequency/time info near cursor
    const auto canvasPoint = screenToCanvas(mousePosition);
    const float freq = screenYToFrequency(mousePosition.y, geom);
    const float time = screenXToTime(mousePosition.x, geom);
    
    g.setFont(createTerminalFont(8.0f));
    g.setColour(RetroColors::TERMINAL_WHITE);
    
    juce::String infoText = juce::String::formatted("%.1fHz | %.2fs", freq, time);
    g.drawText(infoText, 
              mousePosition.x + 10, mousePosition.y - 20,
              80, 15, juce::Justification::left);
}

void RetroCanvasComponent::drawParticleEffect(juce::Graphics& g, juce::Point<float> position, 
                                            float intensity, juce::Colour color)
{
    g.setColour(color.withAlpha(intensity));
    const float size = 2.0f + intensity * 3.0f;
    g.fillEllipse(position.x - size/2, position.y - size/2, size, size);
    
    // Add glow
    g.setColour(color.withAlpha(intensity * 0.3f));
    const float glowSize = size * 2.0f;
    g.fillEllipse(position.x - glowSize/2, position.y - glowSize/2, glowSize, glowSize);
}

void RetroCanvasComponent::drawScanlines(juce::Graphics& g, juce::Rectangle<int> area)
{
    // Draw subtle CRT scanlines for authentic retro feel
    g.setColour(RetroColors::TERMINAL_BLACK.brighter(0.05f));
    
    for (int y = area.getY(); y < area.getBottom(); y += 2)
    {
        g.drawLine(area.getX(), y, area.getRight(), y, 1.0f);
    }
}

//==============================================================================
// Font Management

juce::Font RetroCanvasComponent::createTerminalFont(float size) const
{
    return juce::Font("Courier New", size, juce::Font::plain)
           .withExtraKerningFactor(0.0f);
}

juce::Font RetroCanvasComponent::createBoldTerminalFont(float size) const
{
    return juce::Font("Courier New", size, juce::Font::bold)
           .withExtraKerningFactor(0.0f);
}

//==============================================================================
// Coordinate Conversion

juce::Point<float> RetroCanvasComponent::screenToCanvas(juce::Point<int> screenPoint) const
{
    const auto geom = calculateGeometry();
    
    const float time = screenXToTime(screenPoint.x, geom);
    const float freq = screenYToFrequency(screenPoint.y, geom);
    
    return {time, freq};
}

juce::Point<int> RetroCanvasComponent::canvasToScreen(juce::Point<float> canvasPoint) const
{
    const auto geom = calculateGeometry();
    
    const int x = timeToScreenX(canvasPoint.x, geom);
    const int y = frequencyToScreenY(canvasPoint.y, geom);
    
    return {x, y};
}

float RetroCanvasComponent::screenYToFrequency(int screenY, const CanvasGeometry& geom) const
{
    const float normalizedY = 1.0f - (float)(screenY - geom.canvasArea.getY()) / geom.canvasArea.getHeight();
    const float logMinFreq = std::log2(canvasState.minFreq);
    const float logMaxFreq = std::log2(canvasState.maxFreq);
    const float logFreq = logMinFreq + normalizedY * (logMaxFreq - logMinFreq);
    
    return std::pow(2.0f, logFreq);
}

int RetroCanvasComponent::frequencyToScreenY(float frequency, const CanvasGeometry& geom) const
{
    const float logMinFreq = std::log2(canvasState.minFreq);
    const float logMaxFreq = std::log2(canvasState.maxFreq);
    const float logFreq = std::log2(juce::jlimit(canvasState.minFreq, canvasState.maxFreq, frequency));
    const float normalizedY = (logFreq - logMinFreq) / (logMaxFreq - logMinFreq);
    
    return geom.canvasArea.getY() + (int)((1.0f - normalizedY) * geom.canvasArea.getHeight());
}

float RetroCanvasComponent::screenXToTime(int screenX, const CanvasGeometry& geom) const
{
    return (float)(screenX - geom.canvasArea.getX()) / geom.pixelsPerSecond;
}

int RetroCanvasComponent::timeToScreenX(float time, const CanvasGeometry& geom) const
{
    return geom.canvasArea.getX() + (int)(time * geom.pixelsPerSecond);
}

RetroCanvasComponent::CanvasGeometry RetroCanvasComponent::calculateGeometry() const
{
    if (!geometryNeedsUpdate)
        return cachedGeometry;
    
    CanvasGeometry geom;
    const auto bounds = getLocalBounds();
    
    // Layout the interface elements
    const int freqRulerWidth = 60;
    const int timeRulerHeight = 25;
    const int waveformHeight = 40;
    const int statusBarHeight = 20;
    
    // Status bar at bottom
    geom.statusBar = bounds.removeFromBottom(statusBarHeight);
    
    // Waveform preview above status bar
    geom.waveformArea = bounds.removeFromBottom(waveformHeight);
    
    // Time ruler at bottom of main area
    geom.timeRuler = bounds.removeFromBottom(timeRulerHeight);
    
    // Frequency ruler on left
    geom.freqRuler = bounds.removeFromLeft(freqRulerWidth);
    
    // Remaining area is the main canvas
    geom.canvasArea = bounds;
    
    // Calculate scaling factors
    geom.pixelsPerSecond = geom.canvasArea.getWidth() / canvasState.timeRange * canvasState.zoomLevel;
    geom.pixelsPerOctave = 80.0f * canvasState.zoomLevel;
    
    cachedGeometry = geom;
    geometryNeedsUpdate = false;
    
    return geom;
}

//==============================================================================
// Painting Logic

void RetroCanvasComponent::beginPaintStroke(juce::Point<float> canvasPoint, float pressure)
{
    isPainting = true;
    lastPaintPoint = canvasPoint;
    brushPressure = pressure;
    
    sendPaintCommand(PaintCommandID::BeginStroke, canvasPoint, pressure);
}

void RetroCanvasComponent::updatePaintStroke(juce::Point<float> canvasPoint, float pressure)
{
    if (!isPainting)
        return;
    
    lastPaintPoint = canvasPoint;
    brushPressure = pressure;
    
    sendPaintCommand(PaintCommandID::UpdateStroke, canvasPoint, pressure);
}

void RetroCanvasComponent::endPaintStroke()
{
    if (!isPainting)
        return;
    
    isPainting = false;
    sendPaintCommand(PaintCommandID::EndStroke, {0, 0});
}

void RetroCanvasComponent::sendPaintCommand(PaintCommandID commandID, juce::Point<float> canvasPoint, float pressure)
{
    if (commandTarget)
    {
        Command cmd(commandID, canvasPoint.x, canvasPoint.y, pressure, brushColor);
        commandTarget(cmd);
    }
}

void RetroCanvasComponent::addParticleAt(juce::Point<float> position, juce::Colour color)
{
    Particle particle;
    particle.position = position;
    particle.velocity = {
        (juce::Random::getSystemRandom().nextFloat() - 0.5f) * 20.0f,
        (juce::Random::getSystemRandom().nextFloat() - 0.5f) * 20.0f
    };
    particle.color = color;
    particle.life = 1.0f;
    particle.size = 2.0f + juce::Random::getSystemRandom().nextFloat() * 3.0f;
    
    particles.push_back(particle);
    
    // Limit particle count for performance
    if (particles.size() > 100)
    {
        particles.erase(particles.begin());
    }
}

void RetroCanvasComponent::updateParticles()
{
    const float deltaTime = 1.0f / 60.0f;
    
    for (auto it = particles.begin(); it != particles.end();)
    {
        auto& particle = *it;
        
        // Update position
        particle.position += particle.velocity * deltaTime;
        
        // Update life
        particle.life -= deltaTime * 2.0f; // 0.5 second lifetime
        
        // Apply gravity and friction
        particle.velocity.y += 50.0f * deltaTime; // Gravity
        particle.velocity *= 0.98f; // Friction
        
        // Remove dead particles
        if (particle.life <= 0.0f)
        {
            it = particles.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

//==============================================================================
// Public Interface

void RetroCanvasComponent::setCanvasState(const CanvasState& newState)
{
    canvasState = newState;
    geometryNeedsUpdate = true;
    repaint();
}

void RetroCanvasComponent::clearCanvas()
{
    if (commandTarget)
    {
        Command cmd(PaintCommandID::ClearCanvas);
        commandTarget(cmd);
    }
    
    particles.clear();
    repaint();
}

void RetroCanvasComponent::resetView()
{
    canvasState.zoomLevel = 1.0f;
    canvasState.scrollX = 0.0f;
    canvasState.scrollY = 0.0f;
    geometryNeedsUpdate = true;
    repaint();
}

void RetroCanvasComponent::setBrushType(BrushType type)
{
    currentBrushType = type;
    
    // Set brush-specific colors
    switch (type)
    {
        case BrushType::SineBrush:     brushColor = RetroColors::PAINT_BRIGHT; break;
        case BrushType::HarmonicBrush: brushColor = RetroColors::TERMINAL_CYAN; break;
        case BrushType::NoiseBrush:    brushColor = RetroColors::TERMINAL_RED; break;
        case BrushType::SampleBrush:   brushColor = RetroColors::TERMINAL_BLUE; break;
        case BrushType::GranularPen:   brushColor = RetroColors::TERMINAL_MAGENTA; break;
        case BrushType::CDPMorph:      brushColor = RetroColors::TERMINAL_AMBER; break;
    }
    
    repaint();
}

void RetroCanvasComponent::setPerformanceInfo(float cpuLoad, int activeOscillators, float latency)
{
    currentCPULoad = cpuLoad;
    currentActiveOscillators = activeOscillators;
    currentLatency = latency;
    
    // Only repaint status bar area for performance
    auto geom = calculateGeometry();
    repaint(geom.statusBar);
}