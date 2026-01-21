#pragma once

#include "ModernCyberLookAndFeel.h"
#include "PluginProcessor.h"
#include "VisualizerComponent.h"
#include <JuceHeader.h>

//==============================================================================
class ModulateTab : public juce::Component {
public:
  ModulateTab(HowlingWolvesAudioProcessor &p);
  ~ModulateTab() override;

  void paint(juce::Graphics &g) override;
  void resized() override;

private:
  HowlingWolvesAudioProcessor &audioProcessor;

  // Visualizer
  VisualizerComponent visualizer;

  // Filter Section
  juce::GroupComponent filterGroup;
  juce::Slider cutoffSlider, resSlider;
  juce::ComboBox filterTypeBox;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      cutoffAttachment, resAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
      filterTypeAttachment;
  juce::Label filterLabel;

  // LFO Section
  juce::GroupComponent lfoGroup;
  juce::Slider lfoRateSlider, lfoDepthSlider;
  juce::ComboBox lfoWaveBox, lfoTargetBox;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      lfoRateAttachment, lfoDepthAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
      lfoWaveAttachment, lfoTargetAttachment;
  juce::Label lfoLabel;

  // Helper to setup knobs
  void setupKnob(
      juce::Slider &slider, const juce::String &name,
      std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
          &attachment,
      const juce::String &paramId, bool isBig = false);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulateTab)
};
