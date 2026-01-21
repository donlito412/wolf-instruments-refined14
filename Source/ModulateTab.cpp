#include "ModulateTab.h"

ModulateTab::ModulateTab(HowlingWolvesAudioProcessor &p) : audioProcessor(p) {
  // Visualizer
  addAndMakeVisible(visualizer);

  // Connect visualizer hook
  audioProcessor.audioVisualizerHook =
      [this](const juce::AudioBuffer<float> &buffer) {
        visualizer.pushBuffer(buffer);
      };

  // --- Filter Section ---
  addAndMakeVisible(filterLabel);
  filterLabel.setText("FILTER", juce::dontSendNotification);
  filterLabel.setFont(juce::Font(14.0f, juce::Font::bold));
  filterLabel.setColour(juce::Label::textColourId, WolfColors::ACCENT_CYAN);

  // Only setup attachments if parameters exist
  setupKnob(cutoffSlider, "Cutoff", cutoffAttachment, "filterCutoff", true);
  setupKnob(resSlider, "Res", resAttachment, "filterRes");

  addAndMakeVisible(filterTypeBox);
  filterTypeBox.addItem("Low Pass", 1);
  filterTypeBox.addItem("High Pass", 2);
  filterTypeBox.addItem("Band Pass", 3);
  filterTypeBox.addItem("Notch", 4);
  filterTypeBox.setJustificationType(juce::Justification::centred);

  if (audioProcessor.getAPVTS().getParameter("filterType") != nullptr) {
    filterTypeAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), "filterType", filterTypeBox);
  }

  // --- LFO Section ---
  addAndMakeVisible(lfoLabel);
  lfoLabel.setText("LFO", juce::dontSendNotification);
  lfoLabel.setFont(juce::Font(14.0f, juce::Font::bold));
  lfoLabel.setColour(juce::Label::textColourId, WolfColors::ACCENT_CYAN);

  setupKnob(lfoRateSlider, "Rate", lfoRateAttachment, "lfoRate");
  setupKnob(lfoDepthSlider, "Depth", lfoDepthAttachment, "lfoDepth");

  addAndMakeVisible(lfoWaveBox);
  lfoWaveBox.addItem("Sine", 1);
  lfoWaveBox.addItem("Square", 2);
  lfoWaveBox.addItem("Triangle", 3);
  lfoWaveBox.setJustificationType(juce::Justification::centred);

  if (audioProcessor.getAPVTS().getParameter("lfoWave") != nullptr) {
    lfoWaveAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), "lfoWave", lfoWaveBox);
  }

  addAndMakeVisible(lfoTargetBox);
  lfoTargetBox.addItem("Cutoff", 1);
  lfoTargetBox.addItem("Vol", 2);
  lfoTargetBox.addItem("Pan", 3);
  lfoTargetBox.addItem("Pitch", 4);
  lfoTargetBox.setJustificationType(juce::Justification::centred);

  if (audioProcessor.getAPVTS().getParameter("lfoTarget") != nullptr) {
    lfoTargetAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), "lfoTarget", lfoTargetBox);
  }
}

ModulateTab::~ModulateTab() {
  // Clear hook to prevent crash
  audioProcessor.audioVisualizerHook = nullptr;
}

void ModulateTab::setupKnob(
    juce::Slider &slider, const juce::String &name,
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        &attachment,
    const juce::String &paramId, bool isBig) {
  slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
  slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
  addAndMakeVisible(slider);

  if (audioProcessor.getAPVTS().getParameter(paramId) != nullptr) {
    attachment =
        std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.getAPVTS(), paramId, slider);
  }
}

void ModulateTab::paint(juce::Graphics &g) {
  // Backgrounds for sections
  auto area = getLocalBounds().reduced(20);
  auto topArea = area.removeFromTop(120); // Visualizer area

  area.removeFromTop(20); // Spacer

  auto bottomArea = area;
  auto filterArea =
      bottomArea.removeFromLeft(bottomArea.getWidth() / 2).reduced(10);
  auto lfoArea = bottomArea.reduced(10);

  // Draw Panels
  g.setColour(WolfColors::PANEL_DARK);
  g.fillRoundedRectangle(filterArea.toFloat(), 6.0f);
  g.fillRoundedRectangle(lfoArea.toFloat(), 6.0f);

  g.setColour(WolfColors::BORDER_SUBTLE);
  g.drawRoundedRectangle(filterArea.toFloat(), 6.0f, 1.0f);
  g.drawRoundedRectangle(lfoArea.toFloat(), 6.0f, 1.0f);
}

void ModulateTab::resized() {
  auto area = getLocalBounds().reduced(20);

  // Visualizer (Top)
  visualizer.setBounds(area.removeFromTop(120));

  area.removeFromTop(20); // Spacer

  auto bottomArea = area;
  auto filterArea =
      bottomArea.removeFromLeft(bottomArea.getWidth() / 2).reduced(10);
  auto lfoArea = bottomArea.reduced(10);

  // Filter Layout
  juce::FlexBox filterLayout;
  filterLayout.flexDirection = juce::FlexBox::Direction::column;

  filterLayout.items.add(juce::FlexItem(filterLabel).withHeight(20));

  juce::FlexBox filterControls;
  filterControls.justifyContent = juce::FlexBox::JustifyContent::center;
  filterControls.alignItems = juce::FlexBox::AlignItems::center;

  filterControls.items.add(
      juce::FlexItem(cutoffSlider).withWidth(80).withHeight(80)); // Big Knob
  filterControls.items.add(
      juce::FlexItem(resSlider).withWidth(50).withHeight(50).withMargin(
          {0, 0, 0, 20})); // Small Knob

  filterLayout.items.add(juce::FlexItem(filterControls).withFlex(1));
  filterLayout.items.add(juce::FlexItem(filterTypeBox)
                             .withHeight(25)
                             .withWidth(100)
                             .withMargin({0, 0, 10, 0})
                             .withAlignSelf(juce::FlexItem::AlignSelf::center));

  filterLayout.performLayout(filterArea);

  // LFO Layout
  juce::FlexBox lfoLayout;
  lfoLayout.flexDirection = juce::FlexBox::Direction::column;

  lfoLayout.items.add(juce::FlexItem(lfoLabel).withHeight(20));

  juce::FlexBox lfoControls;
  lfoControls.justifyContent = juce::FlexBox::JustifyContent::center;
  lfoControls.alignItems = juce::FlexBox::AlignItems::center;

  lfoControls.items.add(
      juce::FlexItem(lfoRateSlider).withWidth(60).withHeight(60));
  lfoControls.items.add(juce::FlexItem(lfoDepthSlider)
                            .withWidth(60)
                            .withHeight(60)
                            .withMargin({0, 10, 0, 10}));

  lfoLayout.items.add(juce::FlexItem(lfoControls).withFlex(1));

  juce::FlexBox lfoOptions;
  lfoOptions.justifyContent = juce::FlexBox::JustifyContent::center;
  lfoOptions.items.add(juce::FlexItem(lfoWaveBox)
                           .withHeight(25)
                           .withWidth(80)
                           .withMargin({0, 5, 10, 0}));
  lfoOptions.items.add(juce::FlexItem(lfoTargetBox)
                           .withHeight(25)
                           .withWidth(80)
                           .withMargin({0, 0, 10, 5}));

  lfoLayout.items.add(juce::FlexItem(lfoOptions).withHeight(40));

  lfoLayout.performLayout(lfoArea);
}
