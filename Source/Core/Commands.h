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
    Test,

    // Forge commands
    LoadSample,
    StartPlayback,
    StopPlayback,
    SetPitch,
    SetSpeed,
    SetSyncMode,
    SetVolume,
    SetDrive,
    SetCrush,

    // Canvas commands
    LoadCanvasImage,
    SetCanvasPlayhead,
    SetCanvasActive,
    SetProcessingMode,
    SetCanvasFreqRange
};

// FIFO message object ---------------------------------------------------------
struct Command
{
    ForgeCommandID id = ForgeCommandID::Test;
    int            intParam = -1;        // slot / index / mode
    float          floatParam = 0.0f;    // numeric value
    bool           boolParam = false;    // flag / toggle
    juce::String   stringParam;          // path / text

    Command() = default;
    explicit Command(ForgeCommandID c) : id(c) {}
    Command(ForgeCommandID c, int s) : id(c), intParam(s) {}
    Command(ForgeCommandID c, int s, float v) : id(c), intParam(s), floatParam(v) {}
    Command(ForgeCommandID c, int s, bool  b) : id(c), intParam(s), boolParam(b) {}
    Command(ForgeCommandID c, int s, const juce::String& p) : id(c), intParam(s), stringParam(p) {}

    // Constructor for canvas commands that don't need slot index
    Command(ForgeCommandID c, float v) : id(c), floatParam(v) {}
    Command(ForgeCommandID c, bool b) : id(c), boolParam(b) {}
    Command(ForgeCommandID c, const juce::String& p) : id(c), stringParam(p) {}
};