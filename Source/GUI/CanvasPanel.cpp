#include "GUI/CanvasPanel.h"
#include "GUI/ArtefactLookAndFeel.h"

CanvasPanel::CanvasPanel()
{
    // Set up placeholder label
    placeholderLabel = std::make_unique<juce::Label>("placeholder", "IMAGE CANVAS");
    placeholderLabel->setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 16.0f, juce::Font::plain));
    placeholderLabel->setColour(juce::Label::textColourId, ArtefactLookAndFeel::kTextColour);
    placeholderLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(placeholderLabel.get());
}

void CanvasPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Fill with CRT black
    g.fillAll(ArtefactLookAndFeel::kBackground);

    // Draw inset bevel around entire panel
    g.setColour(ArtefactLookAndFeel::kBevelLight);
    g.drawRect(bounds, 1.0f);

    // Inner content area
    auto contentBounds = bounds.reduced(4);

    if (hasImage && currentImage.isValid())
    {
        // Calculate aspect-correct display bounds
        auto imageAspect = static_cast<float>(currentImage.getWidth()) /
            static_cast<float>(currentImage.getHeight());
        auto panelAspect = static_cast<float>(contentBounds.getWidth()) /
            static_cast<float>(contentBounds.getHeight());

        if (imageAspect > panelAspect)
        {
            // Image is wider - fit to width
            int displayWidth = contentBounds.getWidth();
            int displayHeight = static_cast<int>(displayWidth / imageAspect);
            imageDisplayBounds = juce::Rectangle<float>(
                static_cast<float>(contentBounds.getX()),
                static_cast<float>(contentBounds.getCentreY() - displayHeight / 2),
                static_cast<float>(displayWidth),
                static_cast<float>(displayHeight)
            );
        }
        else
        {
            // Image is taller - fit to height
            int displayHeight = contentBounds.getHeight();
            int displayWidth = static_cast<int>(displayHeight * imageAspect);
            imageDisplayBounds = juce::Rectangle<float>(
                static_cast<float>(contentBounds.getCentreX() - displayWidth / 2),
                static_cast<float>(contentBounds.getY()),
                static_cast<float>(displayWidth),
                static_cast<float>(displayHeight)
            );
        }

        // Draw the image
        g.setOpacity(1.0f);
        g.drawImage(currentImage, imageDisplayBounds);

        // Draw scan lines effect for that CRT feel
        g.setColour(ArtefactLookAndFeel::kBackground.withAlpha(0.3f));
        for (int y = 0; y < getHeight(); y += 2)
        {
            g.drawHorizontalLine(y, 0.0f, static_cast<float>(getWidth()));
        }
    }
    else
    {
        // Draw grid pattern when empty
        g.setColour(ArtefactLookAndFeel::kBevelDark);
        const int gridSize = 32;

        for (int x = contentBounds.getX(); x < contentBounds.getRight(); x += gridSize)
        {
            g.drawVerticalLine(x, static_cast<float>(contentBounds.getY()),
                static_cast<float>(contentBounds.getBottom()));
        }

        for (int y = contentBounds.getY(); y < contentBounds.getBottom(); y += gridSize)
        {
            g.drawHorizontalLine(y, static_cast<float>(contentBounds.getX()),
                static_cast<float>(contentBounds.getRight()));
        }
    }
}

void CanvasPanel::resized()
{
    if (placeholderLabel && !hasImage)
    {
        placeholderLabel->setBounds(getLocalBounds());
    }
}

bool CanvasPanel::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (auto& file : files)
    {
        juce::File f(file);
        if (f.hasFileExtension("jpg;jpeg;png;gif;bmp"))
            return true;
    }
    return false;
}

void CanvasPanel::filesDropped(const juce::StringArray& files, int, int)
{
    if (files.size() > 0)
    {
        loadImage(juce::File(files[0]));
    }
}

void CanvasPanel::loadImage(const juce::File& imageFile)
{
    currentImage = juce::ImageFileFormat::loadFrom(imageFile);
    if (currentImage.isValid())
    {
        currentImageFile = imageFile;
        hasImage = true;
        if (placeholderLabel)
            placeholderLabel->setVisible(false);
        repaint();
    }
}

void CanvasPanel::clearImage()
{
    currentImage = juce::Image();
    currentImageFile = juce::File();
    hasImage = false;
    if (placeholderLabel)
        placeholderLabel->setVisible(true);
    repaint();
}

float CanvasPanel::getBrightnessAt(float normX, float normY) const
{
    if (!hasImage || !currentImage.isValid())
        return 0.0f;
    
    // Convert normalized coords to image pixel coords
    int pixelX = static_cast<int>(normX * currentImage.getWidth());
    int pixelY = static_cast<int>(normY * currentImage.getHeight());
    
    // Clamp to valid range
    pixelX = juce::jlimit(0, currentImage.getWidth() - 1, pixelX);
    pixelY = juce::jlimit(0, currentImage.getHeight() - 1, pixelY);
    
    auto pixel = currentImage.getPixelAt(pixelX, pixelY);
    return pixel.getBrightness();
}