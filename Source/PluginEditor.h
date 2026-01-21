#pragma once

#include "ModernCyberLookAndFeel.h"
#include "PlayTab.h" // Added
#include "PluginProcessor.h"
#include <JuceHeader.h> // PresetBrowser included via PlayTab.h now, or separate if needed (it is in PlayTab.h)

//==============================================================================
// Placeholder tab components (will be implemented in later phases)
//==============================================================================
// PlayTab class definition removed (now in PlayTab.h)

class ModulateTab : public juce::Component {
public:
  ModulateTab() {
    // Placeholder - will implement in Phase 3
  }

  void paint(juce::Graphics &g) override {
    g.fillAll(WolfColors::PANEL_DARK);
    g.setColour(WolfColors::TEXT_PRIMARY);
    g.setFont(20.0f);
    g.drawText("MODULATE TAB - Coming in Phase 3", getLocalBounds(),
               juce::Justification::centred);
  }
};

class EffectsTab : public juce::Component {
public:
  EffectsTab() {
    // Placeholder - will implement in Phase 4
  }

  void paint(juce::Graphics &g) override {
    g.fillAll(WolfColors::PANEL_DARK);
    g.setColour(WolfColors::TEXT_PRIMARY);
    g.setFont(20.0f);
    g.drawText("EFFECTS TAB - Coming in Phase 4", getLocalBounds(),
               juce::Justification::centred);
  }
};

class SettingsTab : public juce::Component {
public:
  SettingsTab() {
    // Placeholder - will implement in Phase 5
  }

  void paint(juce::Graphics &g) override {
    g.fillAll(WolfColors::PANEL_DARK);
    g.setColour(WolfColors::TEXT_PRIMARY);
    g.setFont(20.0f);
    g.drawText("SETTINGS TAB - Coming in Phase 5", getLocalBounds(),
               juce::Justification::centred);
  }
};

//==============================================================================
class HowlingWolvesAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
  HowlingWolvesAudioProcessorEditor(HowlingWolvesAudioProcessor &);
  ~HowlingWolvesAudioProcessorEditor() override;

  void paint(juce::Graphics &) override;
  void resized() override;

private:
  HowlingWolvesAudioProcessor &audioProcessor;

  // Modern UI components
  ModernCyberLookAndFeel modernLookAndFeel;
  juce::TabbedComponent tabs;
  juce::ComponentBoundsConstrainer constrainer;

  // Keyboard (always visible at bottom)
  juce::MidiKeyboardComponent keyboardComponent;

  // Cave background
  juce::Image backgroundImage;

  // Top bar buttons
  juce::TextButton browseButton{"BROWSE"};
  juce::TextButton saveButton{"SAVE"};
  juce::TextButton settingsButton{"?"};

  // Preset browser overlay
  PresetBrowser presetBrowser;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
      HowlingWolvesAudioProcessorEditor)
};
