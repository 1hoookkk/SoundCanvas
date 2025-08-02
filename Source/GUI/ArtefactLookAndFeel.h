#pragma once

#include <JuceHeader.h>

// ==============================================================================
// Brutalist late-90s skin
class ArtefactLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ArtefactLookAndFeel();

    // ------------------------------------------------------------------
    // Palette
    static const juce::Colour kBackground;
    static const juce::Colour kPanelBackground;
    static const juce::Colour kBevelLight;
    static const juce::Colour kBevelDark;
    static const juce::Colour kTextColour;
    static const juce::Colour kAccentColour;
    static const juce::Colour kCanvasBlack;
    static const juce::Colour kWarningRed;
    static const juce::Colour kReadoutGreen;

    // ------------------------------------------------------------------
    juce::Font getSilkscreenFont(float height);

    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
        float sliderPos, float rotaryStart, float rotaryEnd,
        juce::Slider&) override;

    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&,
        bool highlighted, bool down) override;

    void drawTextEditorOutline(juce::Graphics&, int w, int h,
        juce::TextEditor&) override;

    void fillTextEditorBackground(juce::Graphics&, int w, int h,
        juce::TextEditor&) override;

private:
    juce::Typeface::Ptr silkscreenTypeface;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArtefactLookAndFeel)
};
