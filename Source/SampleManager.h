#pragma once

#include "SynthEngine.h"
#include <JuceHeader.h>

//==============================================================================
/**
    Manages loading of samples and mapping them to the synth.
*/
class SampleManager {
public:
  SampleManager(SynthEngine &synth);
  ~SampleManager();

  void loadSamples(); // Initial load (optional)
  void loadSound(const juce::File &file);
  void loadDrumKit(const juce::File &kitDirectory);

private:
  SynthEngine &synthEngine;
  juce::AudioFormatManager formatManager;
};
