#pragma once
// ──────────────────────────────────────────────────────────────────────────────
// JUCE defines a legacy   #define CommandID int
// Undefine it before we declare our own enum:
#ifdef CommandID
#undef CommandID
#endif
// ──────────────────────────────────────────────────────────────────────────────
#include <JuceHeader.h>

// Unique, project-scoped identifiers -----------------------------------------
enum class ForgeCommandID
{
    // Test
    Test = 0,

    // Forge commands
    LoadSample = 10,
    StartPlayback,
    StopPlayback,
    SetPitch,
    SetSpeed,
    SetSyncMode,
    SetVolume,
    SetDrive,
    SetCrush,

    // Canvas commands (legacy - being replaced by PaintCommandID)
    LoadCanvasImage = 50,
    SetCanvasPlayhead,
    SetCanvasActive,
    SetProcessingMode,
    SetCanvasFreqRange
};

// New Paint Engine commands
enum class PaintCommandID
{
    BeginStroke = 200,
    UpdateStroke,
    EndStroke,
    ClearCanvas,
    ClearRegion,
    SetPlayheadPosition,
    SetCanvasRegion,
    SetPaintActive,
    SetMasterGain,
    SetFrequencyRange
};

// FIFO message object ---------------------------------------------------------
struct Command
{
    // Command type - can be either ForgeCommandID or PaintCommandID
    int commandId = static_cast<int>(ForgeCommandID::Test);
    
    // Basic parameters
    int            intParam = -1;        // slot / index / mode
    float          floatParam = 0.0f;    // numeric value
    double         doubleParam = 0.0;    // double precision value
    bool           boolParam = false;    // flag / toggle
    juce::String   stringParam;          // path / text
    
    // Extended parameters for PaintEngine
    float          x = 0.0f;             // Canvas X position
    float          y = 0.0f;             // Canvas Y position  
    float          pressure = 1.0f;      // Brush pressure
    juce::Colour   color;                // Brush color

    // Constructors for Forge commands
    Command() = default;
    explicit Command(ForgeCommandID c) : commandId(static_cast<int>(c)) {}
    Command(ForgeCommandID c, int s) : commandId(static_cast<int>(c)), intParam(s) {}
    Command(ForgeCommandID c, int s, float v) : commandId(static_cast<int>(c)), intParam(s), floatParam(v) {}
    Command(ForgeCommandID c, int s, bool  b) : commandId(static_cast<int>(c)), intParam(s), boolParam(b) {}
    Command(ForgeCommandID c, int s, const juce::String& p) : commandId(static_cast<int>(c)), intParam(s), stringParam(p) {}
    Command(ForgeCommandID c, float v) : commandId(static_cast<int>(c)), floatParam(v) {}
    Command(ForgeCommandID c, bool b) : commandId(static_cast<int>(c)), boolParam(b) {}
    Command(ForgeCommandID c, const juce::String& p) : commandId(static_cast<int>(c)), stringParam(p) {}
    
    // Constructors for Paint commands
    explicit Command(PaintCommandID c) : commandId(static_cast<int>(c)) {}
    Command(PaintCommandID c, float x_, float y_, float pressure_ = 1.0f, juce::Colour color_ = juce::Colours::white)
        : commandId(static_cast<int>(c)), x(x_), y(y_), pressure(pressure_), color(color_) {}
    Command(PaintCommandID c, float x_, float y_, float width, float height)
        : commandId(static_cast<int>(c)), x(x_), y(y_), floatParam(width), doubleParam(height) {}
    Command(PaintCommandID c, float value) : commandId(static_cast<int>(c)), floatParam(value) {}
    Command(PaintCommandID c, bool value) : commandId(static_cast<int>(c)), boolParam(value) {}
    Command(PaintCommandID c, float min, float max) : commandId(static_cast<int>(c)), floatParam(min), doubleParam(max) {}
    
    // Helper methods to check command type
    bool isForgeCommand() const { return commandId < 200; }
    bool isPaintCommand() const { return commandId >= 200; }
    ForgeCommandID getForgeCommandID() const { return static_cast<ForgeCommandID>(commandId); }
    PaintCommandID getPaintCommandID() const { return static_cast<PaintCommandID>(commandId); }
};