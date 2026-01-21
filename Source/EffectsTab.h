#pragma once

#include "ModernCyberLookAndFeel.h"
#include "PluginProcessor.h"
#include <JuceHeader.h>

//==============================================================================
class EffectsTab : public juce::Component {
public:
  EffectsTab(HowlingWolvesAudioProcessor &p);
  ~EffectsTab() override;

  void paint(juce::Graphics &g) override;
  void resized() override;

private:
  HowlingWolvesAudioProcessor &audioProcessor;

  // Distortion
  juce::GroupComponent distGroup;
  juce::Slider distDriveSlider, distMixSlider;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      distDriveAttachment, distMixAttachment;
  juce::Label distLabel;

  // Delay
  juce::GroupComponent delayGroup;
  juce::Slider delayTimeSlider, delayFeedbackSlider, delayMixSlider;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      delayTimeAttachment, delayFeedbackAttachment, delayMixAttachment;
  juce::Label delayLabel;

  // Reverb
  juce::GroupComponent reverbGroup;
  juce::Slider reverbSizeSlider, reverbDampingSlider, reverbMixSlider;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      reverbSizeAttachment, reverbDampingAttachment, reverbMixAttachment;
  juce::Label reverbLabel;

  // Helper
  void setupKnob(
      juce::Slider &slider, const juce::String &name,
      std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
          &attachment,
      const juce::String &paramId);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectsTab)
};
