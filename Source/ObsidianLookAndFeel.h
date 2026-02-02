#pragma once
#include <JuceHeader.h>

class ObsidianLookAndFeel : public juce::LookAndFeel_V4 {
public:
  ObsidianLookAndFeel() {
    setColour(juce::Slider::thumbColourId, juce::Colours::cyan);
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::cyan);
    setColour(juce::Label::textColourId, juce::Colours::silver);

    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1a1a1a));
    setColour(juce::ComboBox::outlineColourId,
              juce::Colours::silver.withAlpha(0.1f));
    setColour(juce::ComboBox::arrowColourId, juce::Colours::cyan);
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff1a1a1a));
    setColour(juce::PopupMenu::textColourId, juce::Colours::silver);
    setColour(juce::PopupMenu::highlightedBackgroundColourId,
              juce::Colours::cyan.withAlpha(0.2f));
  }

  void drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height,
                        float sliderPos, const float rotaryStartAngle,
                        const float rotaryEndAngle,
                        juce::Slider &slider) override {
    auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle =
        rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Fill background
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillEllipse(rx, ry, rw, rw);

    // Border
    g.setColour(juce::Colours::black);
    g.drawEllipse(rx, ry, rw, rw, 1.0f);

    juce::Path p;
    p.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle,
                    angle, true);

    g.setColour(juce::Colours::cyan.withAlpha(0.8f));
    g.strokePath(p, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                         juce::PathStrokeType::rounded));

    if (slider.isEnabled()) {
      g.setColour(juce::Colours::cyan.withAlpha(0.2f));
      g.strokePath(p, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved,
                                           juce::PathStrokeType::rounded));
    }
  }

  // NEW: Linear Slider for Effects Tab
  void drawLinearSlider(juce::Graphics &g, int x, int y, int width, int height,
                        float sliderPos, float minSliderPos, float maxSliderPos,
                        const juce::Slider::SliderStyle style,
                        juce::Slider &slider) override {
    // Only handle horizontal for now as per design
    if (style == juce::Slider::LinearHorizontal) {
      auto trackHeight = 3.0f;
      auto thumbRadius = 5.0f;

      auto centreY = (float)y + (float)height * 0.5f;
      auto trackX = (float)x;
      auto trackW = (float)width;

      // Background Track
      g.setColour(juce::Colours::white.withAlpha(0.1f));
      g.fillRoundedRectangle(trackX, centreY - trackHeight * 0.5f, trackW,
                             trackHeight, 1.5f);

      // Active Track
      g.setColour(juce::Colours::cyan);
      auto activeW = sliderPos - trackX;
      g.fillRoundedRectangle(trackX, centreY - trackHeight * 0.5f, activeW,
                             trackHeight, 1.5f);

      // Thumb
      g.setColour(juce::Colours::cyan);
      g.fillEllipse(sliderPos - thumbRadius, centreY - thumbRadius,
                    thumbRadius * 2.0f, thumbRadius * 2.0f);

      // Thumb Glow
      g.setColour(juce::Colours::cyan.withAlpha(0.3f));
      g.fillEllipse(sliderPos - thumbRadius - 2.0f,
                    centreY - thumbRadius - 2.0f, (thumbRadius + 2.0f) * 2.0f,
                    (thumbRadius + 2.0f) * 2.0f);
    } else {
      // Fallback to default for vertical etc
      juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos,
                                             minSliderPos, maxSliderPos, style,
                                             slider);
    }
  }

  void drawGlassPanel(juce::Graphics &g, juce::Rectangle<int> area) {
    g.setColour(juce::Colours::black.withAlpha(0.7f));
    g.fillRoundedRectangle(area.toFloat(), 10.0f);
    g.setColour(juce::Colours::silver.withAlpha(0.1f));
    g.drawRoundedRectangle(area.toFloat(), 10.0f, 1.5f);
  }
};
