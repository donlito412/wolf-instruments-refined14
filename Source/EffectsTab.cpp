#include "EffectsTab.h"
#include "HuntEngine.h"

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
  setupKnob(reverbMixSlider, "Mix", reverbMixAttachment, "REVERB_MIX");

  // --- Bite ---
  addAndMakeVisible(biteLabel);
  biteLabel.setText("BITE", juce::dontSendNotification);
  biteLabel.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
  biteLabel.setColour(juce::Label::textColourId, WolfColors::ACCENT_CYAN);

  setupKnob(biteSlider, "Bite", biteAttachment, "BITE");

  // --- Hunt ---
  addAndMakeVisible(huntLabel);
  huntLabel.setText("THE HUNT", juce::dontSendNotification);
  huntLabel.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
  huntLabel.setColour(juce::Label::textColourId,
                      WolfColors::ACCENT_RED); // Special color

  addAndMakeVisible(huntModeBox);
  huntModeBox.addItemList(juce::StringArray{"Stalk", "Chase", "Kill"}, 1);
  huntModeBox.setJustificationType(juce::Justification::centred);
  huntModeAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
          audioProcessor.getAPVTS(), "HUNT_MODE", huntModeBox);

  addAndMakeVisible(huntButton);
  huntButton.setButtonText("HUNT");
  huntButton.setColour(juce::TextButton::buttonColourId,
                       WolfColors::ACCENT_RED);
  huntButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

  huntButton.onClick = [this]() {
    // Trigger hunt
    // 1. Get Mode
    int modeIndex = huntModeBox.getSelectedId() - 1; // 1-based to 0-based
    if (modeIndex < 0)
      modeIndex = 0;

    // 2. Call Engine
    auto mode = static_cast<HuntEngine::Mode>(modeIndex);
    audioProcessor.getHuntEngine().hunt(audioProcessor.getAPVTS(), mode);
  };

  // --- Signal Chain ---
  addAndMakeVisible(chainLabel);
  chainLabel.setText("CHAIN", juce::dontSendNotification);
  chainLabel.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
  chainLabel.setColour(juce::Label::textColourId, WolfColors::ACCENT_CYAN);

  addAndMakeVisible(chainBox);
  chainBox.addItemList(
      juce::StringArray{"Standard", "Ethereal", "Chaos", "Reverse"}, 1);
  chainBox.setJustificationType(juce::Justification::centred);
  chainAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
          audioProcessor.getAPVTS(), "CHAIN_ORDER", chainBox);
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

  // Calculate 4 equal columns (approx)
  int colWidth = area.getWidth() / 4;

  auto distArea = area.removeFromLeft(colWidth).reduced(10);
  auto delayArea = area.removeFromLeft(colWidth).reduced(10);
  auto reverbArea = area.removeFromLeft(colWidth).reduced(10);
  // remaining is special area
  auto specialArea = area.reduced(10);

  // Draw Panels
  g.setColour(WolfColors::PANEL_DARK);
  g.fillRoundedRectangle(distArea.toFloat(), 6.0f);
  g.fillRoundedRectangle(delayArea.toFloat(), 6.0f);
  g.fillRoundedRectangle(reverbArea.toFloat(), 6.0f);

  g.setColour(WolfColors::BORDER_SUBTLE);
  g.drawRoundedRectangle(distArea.toFloat(), 6.0f, 1.0f);
  g.drawRoundedRectangle(delayArea.toFloat(), 6.0f, 1.0f);
  g.drawRoundedRectangle(reverbArea.toFloat(), 6.0f, 1.0f);

  // Draw Sub-labels
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

  // Bite
  g.drawText("Bite", biteSlider.getBounds().translated(0, 65),
             juce::Justification::centred);
}

void EffectsTab::resized() {
  auto area = getLocalBounds().reduced(20);

  // Calculate 4 sections
  int colWidth = area.getWidth() / 4;

  auto distArea = area.removeFromLeft(colWidth).reduced(5);
  auto delayArea = area.removeFromLeft(colWidth).reduced(5);
  auto reverbArea = area.removeFromLeft(colWidth).reduced(5);
  auto specialArea = area.reduced(5);

  // --- Distortion Layout ---
  distLabel.setBounds(distArea.removeFromTop(30));
  juce::FlexBox distKnobs;
  distKnobs.justifyContent = juce::FlexBox::JustifyContent::center;
  distKnobs.alignItems = juce::FlexBox::AlignItems::center;
  distKnobs.items.add(
      juce::FlexItem(distDriveSlider).withWidth(70).withHeight(70));
  distKnobs.items.add(juce::FlexItem(distMixSlider)
                          .withWidth(70)
                          .withHeight(70)
                          .withMargin({0, 0, 0, 10}));
  distKnobs.performLayout(distArea);

  // --- Delay Layout ---
  delayLabel.setBounds(delayArea.removeFromTop(30));
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
  reverbLabel.setBounds(reverbArea.removeFromTop(30));
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

  // --- Specials Layout (Bite + Hunt) ---
  // --- Specials Layout (Bite + Hunt + Chain) ---
  // Divide vertical space: Top=Chain, Mid=Bite, Bot=Hunt

  auto chainArea = specialArea.removeFromTop(60);
  chainLabel.setBounds(chainArea.removeFromTop(20));
  chainBox.setBounds(chainArea.reduced(5));

  auto biteArea = specialArea.removeFromTop(specialArea.getHeight() /
                                            2); // Half remaining for Bite
  biteLabel.setBounds(biteArea.removeFromTop(20));
  biteSlider.setBounds(biteArea.reduced(5));

  // Remaining is Hunt
  huntLabel.setBounds(specialArea.removeFromTop(20));

  auto huntControlArea = specialArea;
  huntModeBox.setBounds(huntControlArea.removeFromTop(30).reduced(5));
  huntButton.setBounds(huntControlArea.removeFromBottom(40).reduced(5));
}
