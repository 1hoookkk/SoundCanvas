// GUI/ArtefactLookAndFeel.cpp
#include "ArtefactLookAndFeel.h"

// Define static colors
const juce::Colour ArtefactLookAndFeel::kBackground = juce::Colour(0xff1a1a1a);
const juce::Colour ArtefactLookAndFeel::kPanelBackground = juce::Colour(0xff2a2a2a);
const juce::Colour ArtefactLookAndFeel::kBevelLight = juce::Colour(0xff4a4a4a);
const juce::Colour ArtefactLookAndFeel::kBevelDark = juce::Colour(0xff0a0a0a);
const juce::Colour ArtefactLookAndFeel::kTextColour = juce::Colour(0xffcccccc);
const juce::Colour ArtefactLookAndFeel::kAccentColour = juce::Colour(0xff50a0f0);
const juce::Colour ArtefactLookAndFeel::kCanvasBlack = juce::Colour(0xff000000);
const juce::Colour ArtefactLookAndFeel::kWarningRed = juce::Colour(0xffff4444);
const juce::Colour ArtefactLookAndFeel::kReadoutGreen = juce::Colour(0xff44ff44);

ArtefactLookAndFeel::ArtefactLookAndFeel()
{
    // Set up color scheme
    setColour(juce::ResizableWindow::backgroundColourId, kBackground);
    setColour(juce::Label::textColourId, kTextColour);
    setColour(juce::Slider::textBoxTextColourId, kTextColour);
    setColour(juce::Slider::textBoxBackgroundColourId, kPanelBackground);
    setColour(juce::Slider::textBoxOutlineColourId, kBevelDark);
    setColour(juce::TextButton::buttonColourId, kPanelBackground);
    setColour(juce::TextButton::buttonOnColourId, kAccentColour);
    setColour(juce::TextButton::textColourOffId, kTextColour);
    setColour(juce::TextButton::textColourOnId, kBackground);

    // Try to load custom font (optional - will fall back to default if not found)
    // silkscreenTypeface = juce::Typeface::createSystemTypefaceFor(BinaryData::SilkscreenBold_ttf, 
    //                                                               BinaryData::SilkscreenBold_ttfSize);
}

juce::Font ArtefactLookAndFeel::getSilkscreenFont(float height)
{
    if (silkscreenTypeface != nullptr)
        return juce::Font(silkscreenTypeface).withHeight(height);

    // Fallback to a monospace font
    return juce::Font(juce::Font::getDefaultMonospacedFontName(), height, juce::Font::bold);
}

void ArtefactLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
    juce::Slider& slider)
{
    const float radius = juce::jmin(width / 2, height / 2) - 4.0f;
    const float centreX = x + width * 0.5f;
    const float centreY = y + height * 0.5f;
    const float rx = centreX - radius;
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Background circle
    g.setColour(kPanelBackground);
    g.fillEllipse(rx, ry, rw, rw);

    // Outer ring
    g.setColour(kBevelDark);
    g.drawEllipse(rx, ry, rw, rw, 2.0f);

    // Inner bevel
    g.setColour(kBevelLight.withAlpha(0.3f));
    g.drawEllipse(rx + 2, ry + 2, rw - 4, rw - 4, 1.0f);

    // Pointer
    juce::Path p;
    const float pointerLength = radius * 0.8f;
    const float pointerThickness = 3.0f;
    p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    g.setColour(slider.isEnabled() ? kAccentColour : kTextColour.withAlpha(0.5f));
    g.fillPath(p);
}

void ArtefactLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
    const juce::Colour& backgroundColour,
    bool isMouseOverButton, bool isButtonDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);

    const float cornerSize = 2.0f;
    const bool toggleState = button.getToggleState();

    // Main fill
    g.setColour(toggleState ? kAccentColour :
        isButtonDown ? kPanelBackground.darker(0.2f) :
        isMouseOverButton ? kPanelBackground.brighter(0.1f) :
        kPanelBackground);
    g.fillRoundedRectangle(bounds, cornerSize);

    // Bevel effect
    if (!isButtonDown)
    {
        g.setColour(kBevelLight);
        g.drawLine(bounds.getX(), bounds.getY(), bounds.getRight() - cornerSize, bounds.getY(), 1.0f);
        g.drawLine(bounds.getX(), bounds.getY(), bounds.getX(), bounds.getBottom() - cornerSize, 1.0f);

        g.setColour(kBevelDark);
        g.drawLine(bounds.getX() + cornerSize, bounds.getBottom(), bounds.getRight(), bounds.getBottom(), 1.0f);
        g.drawLine(bounds.getRight(), bounds.getY() + cornerSize, bounds.getRight(), bounds.getBottom(), 1.0f);
    }

    // Border
    g.setColour(kBevelDark);
    g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
}

void ArtefactLookAndFeel::drawTextEditorOutline(juce::Graphics& g, int width, int height,
    juce::TextEditor& textEditor)
{
    if (textEditor.isEnabled())
    {
        g.setColour(textEditor.hasKeyboardFocus(true) ? kAccentColour : kBevelDark);
        g.drawRect(0, 0, width, height, 2);
    }
}

void ArtefactLookAndFeel::fillTextEditorBackground(juce::Graphics& g, int width, int height,
    juce::TextEditor& textEditor)
{
    g.setColour(kCanvasBlack);
    g.fillRect(0, 0, width, height);
}