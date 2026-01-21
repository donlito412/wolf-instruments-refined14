#include "EffectsTab.h"

EffectsTab::EffectsTab(HowlingWolvesAudioProcessor &p) : audioProcessor(p) {
  // --- Distortion ---
  addAndMakeVisible(distLabel);
  distLabel.setText("DISTORTION", juce::dontSendNotification);
  distLabel.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
  distLabel.setColour(juce::Label::textColourId, WolfColors::ACCENT_CYAN);

  setupKnob(distDriveSlider, "Drive", distDriveAttachment, "distDrive");
  setupKnob(distMixSlider, "Mix", distMixAttachment, "distMix");

  // --- Delay ---
  addAndMakeVisible(delayLabel);
  delayLabel.setText("DELAY", juce::dontSendNotification);
  delayLabel.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
  delayLabel.setColour(juce::Label::textColourId, WolfColors::ACCENT_CYAN);

  setupKnob(delayTimeSlider, "Time", delayTimeAttachment, "delayTime");
  setupKnob(delayFeedbackSlider, "Fdbk", delayFeedbackAttachment,
            "delayFeedback");
  setupKnob(delayMixSlider, "Mix", delayMixAttachment, "delayMix");

  // --- Reverb ---
  addAndMakeVisible(reverbLabel);
  reverbLabel.setText("REVERB", juce::dontSendNotification);
  reverbLabel.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
  reverbLabel.setColour(juce::Label::textColourId, WolfColors::ACCENT_CYAN);

  setupKnob(reverbSizeSlider, "Size", reverbSizeAttachment, "reverbSize");
  setupKnob(reverbDampingSlider, "Damp", reverbDampingAttachment,
            "reverbDamping");
  setupKnob(reverbMixSlider, "Mix", reverbMixAttachment, "reverbMix");
}

EffectsTab::~EffectsTab() {}

void EffectsTab::setupKnob(
    juce::Slider &slider, const juce::String &name,
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        &attachment,
    const juce::String &paramId) {
  slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
  slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
  // slider.setTooltip(name); // Optional tooltip
  addAndMakeVisible(slider);

  // Label for knob (simple approach: drawing text in paint for now, or just
  // relying on user knowing layout) Actually, let's just make the slider look
  // good.

  if (audioProcessor.getAPVTS().getParameter(paramId) != nullptr) {
    attachment =
        std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.getAPVTS(), paramId, slider);
  }
}

void EffectsTab::paint(juce::Graphics &g) {
  auto area = getLocalBounds().reduced(20);

  // Calculate 3 equal columns
  auto distArea = area.removeFromLeft(area.getWidth() / 3).reduced(10);
  auto delayArea = area.removeFromLeft(area.getWidth() / 2).reduced(10);
  auto reverbArea = area.reduced(10);

  // Draw Panels
  g.setColour(WolfColors::PANEL_DARK);
  g.fillRoundedRectangle(distArea.toFloat(), 6.0f);
  g.fillRoundedRectangle(delayArea.toFloat(), 6.0f);
  g.fillRoundedRectangle(reverbArea.toFloat(), 6.0f);

  g.setColour(WolfColors::BORDER_SUBTLE);
  g.drawRoundedRectangle(distArea.toFloat(), 6.0f, 1.0f);
  g.drawRoundedRectangle(delayArea.toFloat(), 6.0f, 1.0f);
  g.drawRoundedRectangle(reverbArea.toFloat(), 6.0f, 1.0f);

  // Draw Sub-labels (manual for now for simplicity)
  g.setColour(WolfColors::TEXT_SECONDARY);
  g.setFont(12.0f);

  // Dist
  g.drawText("Drive", distDriveSlider.getBounds().translated(0, 70),
             juce::Justification::centred);
  g.drawText("Mix", distMixSlider.getBounds().translated(0, 70),
             juce::Justification::centred);

  // Delay
  g.drawText("Time", delayTimeSlider.getBounds().translated(0, 65),
             juce::Justification::centred);
  g.drawText("Fdbk", delayFeedbackSlider.getBounds().translated(0, 65),
             juce::Justification::centred);
  g.drawText("Mix", delayMixSlider.getBounds().translated(0, 65),
             juce::Justification::centred);

  // Reverb
  g.drawText("Size", reverbSizeSlider.getBounds().translated(0, 65),
             juce::Justification::centred);
  g.drawText("Damp", reverbDampingSlider.getBounds().translated(0, 65),
             juce::Justification::centred);
  g.drawText("Mix", reverbMixSlider.getBounds().translated(0, 65),
             juce::Justification::centred);
}

void EffectsTab::resized() {
  auto area = getLocalBounds().reduced(20);

  // Calculate 3 equal columns
  auto distArea = area.removeFromLeft(area.getWidth() / 3).reduced(10);
  auto delayArea = area.removeFromLeft(area.getWidth() / 2).reduced(10);
  auto reverbArea = area.reduced(10);

  // --- Distortion Layout ---
  // 1. Label
  distLabel.setBounds(distArea.removeFromTop(30));

  // 2. Knobs (Row)
  juce::FlexBox distKnobs;
  distKnobs.justifyContent = juce::FlexBox::JustifyContent::center;
  distKnobs.alignItems = juce::FlexBox::AlignItems::center;
  distKnobs.items.add(
      juce::FlexItem(distDriveSlider).withWidth(70).withHeight(70));
  distKnobs.items.add(juce::FlexItem(distMixSlider)
                          .withWidth(70)
                          .withHeight(70)
                          .withMargin({0, 0, 0, 10}));

  distKnobs.performLayout(distArea); // Layout in remaining space

  // --- Delay Layout ---
  // 1. Label
  delayLabel.setBounds(delayArea.removeFromTop(30));

  // 2. Knobs (Row/Wrap)
  juce::FlexBox delayKnobs;
  delayKnobs.justifyContent = juce::FlexBox::JustifyContent::center;
  delayKnobs.alignItems = juce::FlexBox::AlignItems::center;
  delayKnobs.flexWrap = juce::FlexBox::Wrap::wrap;
  delayKnobs.items.add(juce::FlexItem(delayTimeSlider)
                           .withWidth(60)
                           .withHeight(60)
                           .withMargin(5));
  delayKnobs.items.add(juce::FlexItem(delayFeedbackSlider)
                           .withWidth(60)
                           .withHeight(60)
                           .withMargin(5));
  delayKnobs.items.add(juce::FlexItem(delayMixSlider)
                           .withWidth(60)
                           .withHeight(60)
                           .withMargin(5));

  delayKnobs.performLayout(delayArea);

  // --- Reverb Layout ---
  // 1. Label
  reverbLabel.setBounds(reverbArea.removeFromTop(30));

  // 2. Knobs (Row/Wrap)
  juce::FlexBox reverbKnobs;
  reverbKnobs.justifyContent = juce::FlexBox::JustifyContent::center;
  reverbKnobs.alignItems = juce::FlexBox::AlignItems::center;
  reverbKnobs.flexWrap = juce::FlexBox::Wrap::wrap;
  reverbKnobs.items.add(juce::FlexItem(reverbSizeSlider)
                            .withWidth(60)
                            .withHeight(60)
                            .withMargin(5));
  reverbKnobs.items.add(juce::FlexItem(reverbDampingSlider)
                            .withWidth(60)
                            .withHeight(60)
                            .withMargin(5));
  reverbKnobs.items.add(juce::FlexItem(reverbMixSlider)
                            .withWidth(60)
                            .withHeight(60)
                            .withMargin(5));

  reverbKnobs.performLayout(reverbArea);
}
