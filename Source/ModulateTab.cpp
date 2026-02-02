#include "ModulateTab.h"

ModulateTab::ModulateTab(HowlingWolvesAudioProcessor &p) : audioProcessor(p) {
  // --- 1. LFO VISUALIZER (TOP) ---
  setupLabel(visTitle, "LFO 1 VISUALIZER");
  setupLabel(syncLabel, "SYNC: 1/4");

  // --- 2. LFO PARAMETERS (LEFT PANEL) ---
  setupLabel(lfoTitle, "LFO PARAMETERS");

  addAndMakeVisible(waveSelector);
  waveSelector.addItemList(
      {"SINE", "SQUARE", "TRIANGLE"},
      1); // Matching Processor choices order usually: Sine, Square, Triangle
  // User Snippet: {"SINE", "TRIANGLE", "SAW", "SQUARE"}.
  // Processor: Sine, Square, Triangle.
  // I will stick to Processor choices to ensure correct mapping if I attach.
  // Actually, I'll match the User Snippet visuals but mapping might be slightly
  // off if not updated in Processor. I'll stick to Processor Layout: Sine,
  // Square, Triangle.

  if (auto *a = audioProcessor.getAPVTS().getParameter("lfoWave"))
    waveAtt = std::make_unique<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), "lfoWave", waveSelector);
  else
    waveSelector.setSelectedId(1);

  setupKnob(rateKnob, "RATE", "lfoRate", rateAtt);
  setupKnob(depthKnob, "DEPTH", "lfoDepth", depthAtt);

  // Phase and Smooth don't exist in processor yet, just Visuals
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> nullAtt;
  setupKnob(phaseKnob, "PHASE", "", nullAtt);
  setupSlider(smoothSlider, "SMOOTH", true, "", nullAtt);

  // --- 3. MODULATION ROUTING (RIGHT PANEL) ---
  setupLabel(routingTitle, "MODULATION ROUTING");
  addAndMakeVisible(targetSelector);
  targetSelector.addItemList({"FILTER CUTOFF", "VOLUME", "PAN", "PITCH"},
                             1); // Matching Processor Choices

  if (auto *a = audioProcessor.getAPVTS().getParameter("lfoTarget"))
    targetAtt = std::make_unique<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), "lfoTarget", targetSelector);
  else
    targetSelector.setSelectedId(1);

  for (auto *s : {&modA, &modD, &modS, &modR}) {
    // Visual dummies for LFO Envelope? Or Mod Envelope?
    setupSlider(*s, "Env", false, "", nullAtt);
  }
  setupSlider(amountSlider, "MOD AMOUNT", true, "", nullAtt);

  startTimerHz(60);
}

ModulateTab::~ModulateTab() { stopTimer(); }

void ModulateTab::timerCallback() {
  phaseOffset += 0.05f;
  repaint();
}

void ModulateTab::setupKnob(
    juce::Slider &s, const juce::String &name, const juce::String &paramId,
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        &att) {
  addAndMakeVisible(s);
  s.setSliderStyle(juce::Slider::Rotary);
  s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
  if (audioProcessor.getAPVTS().getParameter(paramId))
    att =
        std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.getAPVTS(), paramId, s);
}

void ModulateTab::setupSlider(
    juce::Slider &s, const juce::String &name, bool horizontal,
    const juce::String &paramId,
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        &att) {
  addAndMakeVisible(s);
  s.setSliderStyle(horizontal ? juce::Slider::LinearHorizontal
                              : juce::Slider::LinearVertical);
  s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
  if (audioProcessor.getAPVTS().getParameter(paramId))
    att =
        std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.getAPVTS(), paramId, s);
}

void ModulateTab::setupLabel(juce::Label &l, const juce::String &t) {
  addAndMakeVisible(l);
  l.setText(t, juce::dontSendNotification);
  l.setColour(juce::Label::textColourId, juce::Colours::silver);
  l.setFont(juce::Font(12.0f, juce::Font::bold));
}

void ModulateTab::drawLFOWave(juce::Graphics &g, juce::Rectangle<int> area) {
  g.setColour(juce::Colours::black.withAlpha(0.2f));
  g.fillRoundedRectangle(area.toFloat(), 6.0f);

  juce::Path wave;
  float midY = (float)area.getCentreY();
  wave.startNewSubPath((float)area.getX(), midY);
  for (float x = 0; x < area.getWidth(); x += 2.0f) {
    float y =
        midY + std::sin(x * 0.04f + phaseOffset) * (area.getHeight() * 0.3f);
    wave.lineTo((float)area.getX() + x, y);
  }
  g.setColour(juce::Colours::cyan);
  g.strokePath(wave, juce::PathStrokeType(2.5f));
}

void ModulateTab::paint(juce::Graphics &g) {
  auto *lnf = dynamic_cast<ObsidianLookAndFeel *>(&getLookAndFeel());
  if (lnf) {
    lnf->drawGlassPanel(g, visPanel);
    lnf->drawGlassPanel(g, lfoPanel);
    lnf->drawGlassPanel(g, routingPanel);
  } else {
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    for (auto area : {visPanel, lfoPanel, routingPanel})
      g.fillRoundedRectangle(area.toFloat(), 10.0f);
  }

  // Draw LFO Wave Visualizer
  drawLFOWave(g, visPanel.reduced(15));
}

void ModulateTab::resized() {
  auto area = getLocalBounds().reduced(15);

  // Top section for Visualizer
  visPanel = area.removeFromTop((int)(getHeight() * 0.35f)).reduced(5);
  visTitle.setBounds(visPanel.getX() + 10, visPanel.getY() + 5, 200, 30);
  syncLabel.setBounds(visPanel.getRight() - 110, visPanel.getY() + 5, 100, 30);

  // Bottom section split for LFO and Routing
  auto bottomArea = area.reduced(0, 10);
  lfoPanel = bottomArea.removeFromLeft((int)(bottomArea.getWidth() * 0.48f))
                 .reduced(5);
  routingPanel =
      bottomArea.removeFromRight((int)(bottomArea.getWidth() * 0.94f))
          .reduced(5);
  // Note: User snippet said *0.94f after removing left...
  // If left is ~48%, remaining is 52%. 94% of 52% is ~49%. Matches nicely
  // roughly half-half. Or did user mean "removeFromRight(remaining.width
  // * 1.0)"? 0.94 seems specific or maybe a typo for "rest". I'll stick to
  // snippet logic.

  // --- LFO PARAMETERS LAYOUT ---
  auto lArea = lfoPanel.reduced(15);
  lfoTitle.setBounds(lArea.removeFromTop(30));
  waveSelector.setBounds(lArea.removeFromTop(35).reduced(20, 0));

  // Center Knobs row
  auto knobRow = lArea.removeFromTop(100);
  int kw = knobRow.getWidth() / 3;
  rateKnob.setBounds(knobRow.removeFromLeft(kw).withSizeKeepingCentre(60, 60));
  depthKnob.setBounds(knobRow.removeFromLeft(kw).withSizeKeepingCentre(60, 60));
  phaseKnob.setBounds(knobRow.withSizeKeepingCentre(60, 60));

  smoothSlider.setBounds(lArea.removeFromBottom(40).reduced(20, 0));

  // --- ROUTING LAYOUT ---
  auto rArea = routingPanel.reduced(15);
  routingTitle.setBounds(rArea.removeFromTop(30));
  targetSelector.setBounds(rArea.removeFromTop(35).reduced(20, 0));

  amountSlider.setBounds(rArea.removeFromBottom(40).reduced(20, 0));

  auto adsrArea = rArea.reduced(30, 10);
  float w = (float)adsrArea.getWidth() / 4.0f;

  auto placeSlider = [&](juce::Slider &s) {
    s.setBounds(adsrArea.removeFromLeft((int)w).reduced(5, 0));
  };

  placeSlider(modA);
  placeSlider(modD);
  placeSlider(modS);
  placeSlider(modR);
}
