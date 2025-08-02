// GUI/ArtefactLookAndFeel.cpp
#include "ArtefactLookAndFeel.h"

// Define static colors - RADIOACTIVE VOMIT PALETTE 🌈💀
const juce::Colour ArtefactLookAndFeel::kBackground = juce::Colour(0xffff00ff);        // Magenta background
const juce::Colour ArtefactLookAndFeel::kPanelBackground = juce::Colour(0xff00ff00);  // Lime green panels  
const juce::Colour ArtefactLookAndFeel::kBevelLight = juce::Colour(0xffffff00);       // Yellow highlights
const juce::Colour ArtefactLookAndFeel::kBevelDark = juce::Colour(0xff800080);        // Purple shadows
const juce::Colour ArtefactLookAndFeel::kTextColour = juce::Colour(0xffffff00);       // Yellow text
const juce::Colour ArtefactLookAndFeel::kAccentColour = juce::Colour(0xffff0000);     // Bright red accent
const juce::Colour ArtefactLookAndFeel::kCanvasBlack = juce::Colour(0xff00ffff);      // Cyan "black"
const juce::Colour ArtefactLookAndFeel::kWarningRed = juce::Colour(0xff00ffff);       // Cyan "red" 
const juce::Colour ArtefactLookAndFeel::kReadoutGreen = juce::Colour(0xff0000ff);     // Blue "green"

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

juce::Font ArtefactLookAndFeel::getRandomCursedFont(float height)
{
    // Gemini's cursed font collection 💀
    static std::vector<juce::String> cursedFonts = {
        "Comic Sans MS", "Papyrus", "Jokerman", "Chiller", 
        "Curlz MT", "Kristen ITC", "Gigi", "Impact",
        "Webdings", "Wingdings", "Brush Script MT"
    };
    
    // Better randomness as Gemini suggested
    static juce::Random randomizer;
    auto fontIndex = randomizer.nextInt(cursedFonts.size());
    auto chosenFont = cursedFonts[fontIndex];
    
    // Random styling because why not
    auto style = (fontIndex % 3 == 0) ? juce::Font::bold : 
                 (fontIndex % 3 == 1) ? juce::Font::italic : juce::Font::plain;
    
    return juce::Font(chosenFont, height, style);
}

juce::Font ArtefactLookAndFeel::getForumFont(float height)
{
    // Classic early 2000s forum fonts
    static std::vector<juce::String> forumFonts = {
        "Arial", "Times New Roman", "Verdana", "Georgia", "Trebuchet MS"
    };
    
    // But make them weird
    auto fontIndex = (int)(height * 13.37f) % forumFonts.size();  // Cursed math
    return juce::Font(forumFonts[fontIndex], height, juce::Font::bold);
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

    // Amateur forum-style random corner sizes
    const float cornerSize = (button.getComponentID().getHashCode() % 15) + 2.0f;
    const bool toggleState = button.getToggleState();

    // CURSED GRADIENT ABUSE 🌈
    juce::ColourGradient gradient;
    if (isButtonDown)
    {
        // Seizure-inducing pressed state
        gradient = juce::ColourGradient::vertical(kAccentColour, kWarningRed, bounds);
        gradient.addColour(0.3, kReadoutGreen);
        gradient.addColour(0.7, kBevelLight);
    }
    else if (isMouseOverButton)
    {
        // Hover state that hurts your eyes
        gradient = juce::ColourGradient::horizontal(kPanelBackground, kCanvasBlack, bounds);
        gradient.addColour(0.5, kAccentColour.withAlpha(0.8f));
    }
    else
    {
        // Default state - still cursed
        gradient = juce::ColourGradient::vertical(kPanelBackground, kBevelDark, bounds);
        gradient.addColour(0.25, kTextColour.withAlpha(0.3f));
        gradient.addColour(0.75, kReadoutGreen.withAlpha(0.2f));
    }
    
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, cornerSize);

    // Amateur hour bevel effect (but make it worse)
    if (!isButtonDown)
    {
        // Top-left highlight (wrong colors on purpose)
        g.setColour(kWarningRed.withAlpha(0.6f));
        g.drawLine(bounds.getX(), bounds.getY(), bounds.getRight() - cornerSize, bounds.getY(), 2.0f);
        g.drawLine(bounds.getX(), bounds.getY(), bounds.getX(), bounds.getBottom() - cornerSize, 2.0f);

        // Bottom-right shadow (also wrong colors)
        g.setColour(kReadoutGreen.withAlpha(0.4f));
        g.drawLine(bounds.getX() + cornerSize, bounds.getBottom(), bounds.getRight(), bounds.getBottom(), 3.0f);
        g.drawLine(bounds.getRight(), bounds.getY() + cornerSize, bounds.getRight(), bounds.getBottom(), 3.0f);
    }

    // Random border thickness because consistency is for professionals
    float borderThickness = (button.getWidth() % 3) + 1.0f;
    g.setColour(kAccentColour);
    g.drawRoundedRectangle(bounds, cornerSize, borderThickness);
    
    // Extra cursed detail: random dots
    g.setColour(kTextColour.withAlpha(0.3f));
    for (int i = 0; i < 3; ++i)
    {
        float dotX = bounds.getX() + (button.getComponentID().getHashCode() * (i + 1)) % (int)bounds.getWidth();
        float dotY = bounds.getY() + (button.getHeight() * i / 4);
        g.fillEllipse(dotX, dotY, 2, 2);
    }
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

void ArtefactLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float minSliderPos, float maxSliderPos,
    const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    // Amateur forum slider design
    auto trackArea = juce::Rectangle<float>(x, y, width, height);
    
    // Horrible track background with cursed gradient
    juce::ColourGradient trackGradient = juce::ColourGradient::horizontal(
        kWarningRed.withAlpha(0.6f), kReadoutGreen.withAlpha(0.8f), trackArea);
    trackGradient.addColour(0.5, kAccentColour.withAlpha(0.4f));
    g.setGradientFill(trackGradient);
    g.fillRoundedRectangle(trackArea, 3.0f);
    
    // Track outline that clashes
    g.setColour(kTextColour);
    g.drawRoundedRectangle(trackArea, 3.0f, 2.0f);
    
    // Quantum slider behavior - randomly offset the thumb position
    static juce::Random quantumChaos;
    float chaosOffset = quantumChaos.nextFloat() * 6.0f - 3.0f; // ±3 pixels of chaos
    float cursedSliderPos = sliderPos + chaosOffset;
    
    // Thumb (way too big and wrong colors)
    float thumbSize = 20.0f;
    juce::Rectangle<float> thumbArea;
    
    if (style == juce::Slider::LinearHorizontal)
    {
        thumbArea = juce::Rectangle<float>(cursedSliderPos - thumbSize/2, 
                                         y + height/2 - thumbSize/2, 
                                         thumbSize, thumbSize);
    }
    else
    {
        thumbArea = juce::Rectangle<float>(x + width/2 - thumbSize/2,
                                         cursedSliderPos - thumbSize/2,
                                         thumbSize, thumbSize);
    }
    
    // Cursed thumb gradient
    juce::ColourGradient thumbGradient = juce::ColourGradient::radial(
        thumbArea.getCentreX(), thumbArea.getCentreY(), thumbSize/2,
        kAccentColour, kBevelDark);
    thumbGradient.addColour(0.3, kReadoutGreen);
    thumbGradient.addColour(0.7, kWarningRed.withAlpha(0.9f));
    
    g.setGradientFill(thumbGradient);
    g.fillEllipse(thumbArea);
    
    // Amateur hour border
    g.setColour(kTextColour);
    g.drawEllipse(thumbArea, 3.0f);
    
    // Random decorative elements because why not
    g.setColour(kBevelLight.withAlpha(0.5f));
    for (int i = 0; i < 5; ++i)
    {
        float dotX = x + (quantumChaos.nextFloat() * width);
        float dotY = y + (quantumChaos.nextFloat() * height); 
        g.fillEllipse(dotX, dotY, 2, 2);
    }
}