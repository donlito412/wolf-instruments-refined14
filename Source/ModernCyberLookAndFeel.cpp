#include "ModernCyberLookAndFeel.h"

ModernCyberLookAndFeel::ModernCyberLookAndFeel() {
  // Set default colors
  setColour(juce::ResizableWindow::backgroundColourId, WolfColors::BG_CAVE);
  setColour(juce::TextButton::buttonColourId, WolfColors::CONTROL_BG);
  setColour(juce::TextButton::textColourOffId, WolfColors::TEXT_PRIMARY);
  setColour(juce::TextButton::textColourOnId, juce::Colours::white);
  setColour(juce::Slider::backgroundColourId, WolfColors::CONTROL_BG);
  setColour(juce::Slider::thumbColourId, WolfColors::TEXT_PRIMARY);
  setColour(juce::Slider::trackColourId, WolfColors::ACCENT_CYAN);
}

void ModernCyberLookAndFeel::drawTabButton(juce::TabBarButton &button,
                                           juce::Graphics &g, bool isMouseOver,
                                           bool isMouseDown) {
  auto area = button.getActiveArea().toFloat();
  auto isActive = button.getToggleState();

  // Background (only for active tab)
  if (isActive) {
    g.setColour(WolfColors::PANEL_DARK);
    g.fillRoundedRectangle(area, 3.0f);
  }

  // Bottom border for active tab (cyan accent)
  if (isActive) {
    g.setColour(WolfColors::ACCENT_CYAN);
    g.fillRect(area.getX(), area.getBottom() - 2.0f, area.getWidth(), 2.0f);
  }

  // Text
  g.setColour(isActive ? WolfColors::ACCENT_CYAN : WolfColors::TEXT_SECONDARY);
  g.setFont(12.0f);
  g.drawText(button.getButtonText(), area, juce::Justification::centred);
}

void ModernCyberLookAndFeel::drawTabbedButtonBarBackground(
    juce::TabbedButtonBar &bar, juce::Graphics &g) {
  // Dark background for tab bar
  g.setColour(WolfColors::PANEL_DARKER);
  g.fillRect(bar.getLocalBounds());

  // Subtle bottom border
  g.setColour(WolfColors::BORDER_SUBTLE);
  g.fillRect(0, bar.getHeight() - 1, bar.getWidth(), 1);
}

int ModernCyberLookAndFeel::getTabButtonBestWidth(juce::TabBarButton &button,
                                                  int tabDepth) {
  return 120; // Fixed width for clean appearance
}

void ModernCyberLookAndFeel::drawButtonBackground(
    juce::Graphics &g, juce::Button &button,
    const juce::Colour &backgroundColour, bool isMouseOverButton,
    bool isButtonDown) {
  auto bounds = button.getLocalBounds().toFloat();
  auto isToggled = button.getToggleState();

  // Background
  g.setColour(isButtonDown        ? WolfColors::ACCENT_TEAL
              : isToggled         ? WolfColors::ACCENT_CYAN
              : isMouseOverButton ? WolfColors::CONTROL_BG.brighter(0.1f)
                                  : backgroundColour);
  g.fillRoundedRectangle(bounds, 3.0f);

  // Subtle border
  g.setColour(WolfColors::BORDER_SUBTLE);
  g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
}

void ModernCyberLookAndFeel::drawLinearSlider(
    juce::Graphics &g, int x, int y, int width, int height, float sliderPos,
    float minSliderPos, float maxSliderPos,
    const juce::Slider::SliderStyle style, juce::Slider &slider) {
  if (style == juce::Slider::LinearHorizontal) {
    // Track background
    auto trackY = y + height / 2 - 2;
    g.setColour(WolfColors::CONTROL_BG);
    g.fillRoundedRectangle(x, trackY, width, 4, 2.0f);

    // Cyan fill from start to slider position
    g.setColour(WolfColors::ACCENT_CYAN);
    g.fillRoundedRectangle(x, trackY, sliderPos - x, 4, 2.0f);

    // Thumb (small circle)
    g.setColour(WolfColors::TEXT_PRIMARY);
    g.fillEllipse(sliderPos - 6, y + height / 2 - 6, 12, 12);

    // Subtle thumb border
    g.setColour(WolfColors::ACCENT_CYAN);
    g.drawEllipse(sliderPos - 6, y + height / 2 - 6, 12, 12, 1.0f);
  }
}

void ModernCyberLookAndFeel::drawRotarySlider(
    juce::Graphics &g, int x, int y, int width, int height, float sliderPos,
    float rotaryStartAngle, float rotaryEndAngle, juce::Slider &slider) {
  auto radius = juce::jmin(width / 2, height / 2) - 4.0f;
  auto centreX = x + width * 0.5f;
  auto centreY = y + height * 0.5f;
  auto angle =
      rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

  // Background circle
  g.setColour(WolfColors::CONTROL_BG);
  g.fillEllipse(centreX - radius, centreY - radius, radius * 2, radius * 2);

  // Progress arc (cyan, 2px stroke, no glow)
  juce::Path arc;
  arc.addCentredArc(centreX, centreY, radius - 2, radius - 2, 0.0f,
                    rotaryStartAngle, angle, true);
  g.setColour(WolfColors::ACCENT_CYAN);
  g.strokePath(arc, juce::PathStrokeType(2.0f));

  // Center indicator line
  juce::Path indicator;
  indicator.addLineSegment(juce::Line<float>(centreX, centreY - radius * 0.4f,
                                             centreX, centreY - radius * 0.7f),
                           2.0f);
  indicator.applyTransform(
      juce::AffineTransform::rotation(angle, centreX, centreY));
  g.setColour(WolfColors::TEXT_PRIMARY);
  g.fillPath(indicator);

  // Subtle border
  g.setColour(WolfColors::BORDER_SUBTLE);
  g.drawEllipse(centreX - radius, centreY - radius, radius * 2, radius * 2,
                1.0f);
}
