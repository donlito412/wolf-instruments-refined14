#include "SampleManager.h"

SampleManager::SampleManager(SynthEngine &s) : synthEngine(s) {
  formatManager.registerBasicFormats();
}

SampleManager::~SampleManager() {}

// Helper to get standard location: ~/Music/Wolf Instruments/Howling
// Wolves/Samples
// Helper to get standard location: ~/Music/Wolf Instruments/Howling
// Wolves/Samples
// Helper to get standard location with priority search
// REMOVED per user request

void SampleManager::loadSamples() {
  // Initial load can be left empty or load a default welcome sound.
  // We rely on the user selecting a preset.
  synthEngine.clearSounds();
}

void SampleManager::loadSound(const juce::File &file) {
  if (!file.existsAsFile())
    return;

  // Clear current sounds first so we don't play the old one if this load fails
  synthEngine.clearSounds();

  std::unique_ptr<juce::AudioFormatReader> reader(
      formatManager.createReaderFor(file));

  if (reader != nullptr) {
    juce::BigInteger allNotes;
    allNotes.setRange(0, 128, true);

    // Try to read root note from metadata (smpl chunk)
    int rootNote = 60; // C3 default
    if (reader->metadataValues.containsKey("RootNote")) {
      rootNote = reader->metadataValues["RootNote"].getIntValue();
    }

    auto *sound = new HowlingSound(file.getFileNameWithoutExtension(), *reader,
                                   allNotes, rootNote, 0.0, 100.0, 60.0);

    synthEngine.addSound(sound);
  } else {
    DBG("Failed to load sample: " + file.getFullPathName());
  }
}

void SampleManager::loadDrumKit(const juce::File &kitDirectory) {
  if (!kitDirectory.isDirectory())
    return;

  // Clear existing sounds (Kit replaces current set)
  synthEngine.clearSounds();

  auto allowedExtensions = formatManager.getWildcardForAllFormats();
  int midiNote = 36; // Start at C1 (Standard Drum Map)
  int count = 0;

  // Iterate files in the directory
  for (const auto &file : kitDirectory.findChildFiles(
           juce::File::findFiles, false, allowedExtensions)) {

    if (count >= 16)
      break; // Limit to 16 pads

    std::unique_ptr<juce::AudioFormatReader> reader(
        formatManager.createReaderFor(file));

    if (reader != nullptr) {
      // Map to SINGLE note
      juce::BigInteger noteMap;
      noteMap.setBit(midiNote);

      auto *sound =
          new HowlingSound(file.getFileNameWithoutExtension(), *reader, noteMap,
                           midiNote,        // Root note = played note
                           0.0, 0.1, 60.0); // Fast attack

      synthEngine.addSound(sound);
      midiNote++;
      count++;
    }
  }
}
