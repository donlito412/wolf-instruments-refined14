#include "MidiProcessor.h"

//==============================================================================
// Arpeggiator
//==============================================================================

Arpeggiator::Arpeggiator() {
  // Init sequence to -1 (empty) or 0 (root note)
  // Let's init to a simple pattern like Root every quarter?
  // Or just empty. User wants to "switch to step sequencer".
  // Let's init to -1.
  sequence.fill(-1);
}

void Arpeggiator::prepare(double sampleRate) { currentSampleRate = sampleRate; }

void Arpeggiator::reset() {
  sortedNotes.clear();
  currentStep = 0;
  noteTime = 0.0;
  // Panic: Clear active notes to stop stuck ones on reset
  activeNotes.clear();
}

void Arpeggiator::setParameters(float rate, int mode, int octaves, float gate,
                                bool on, float density, float complexity,
                                float spread) {
  rateDiv = rate;
  arpMode = mode;
  numOctaves = octaves;
  gateLength = gate;
  enabled = on;
  arpDensity = density;       // 0.0-1.0
  arpComplexity = complexity; // 0.0-1.0
  arpSpread = spread;
}

void Arpeggiator::setRhythmStep(int step, int semitoneOffset) {
  if (step >= 0 && step < 16)
    sequence[(size_t)step] = semitoneOffset;
}

int Arpeggiator::getRhythmStep(int step) const {
  if (step >= 0 && step < 16)
    return sequence[(size_t)step];
  return -1;
}

void Arpeggiator::handleNoteOn(int note, int velocity) {
  bool wasEmpty = sortedNotes.empty();

  // Add unique and sort
  bool found = false;
  for (int n : sortedNotes)
    if (n == note)
      found = true;

  if (!found) {
    sortedNotes.push_back(note);
    std::sort(sortedNotes.begin(), sortedNotes.end());
  }

  // Instant Trigger: If this is the first note, force trigger ASAP
  if (wasEmpty) {
    currentStep = 0;
    noteTime = 1000000.0;
  }
}

void Arpeggiator::handleNoteOff(int note) {
  // Remove
  auto it = std::remove(sortedNotes.begin(), sortedNotes.end(), note);
  sortedNotes.erase(it, sortedNotes.end());
}

// Helper to check if grid is empty
bool Arpeggiator::isGridEmpty() const {
  for (int s : sequence) {
    if (s != -1)
      return false;
  }
  return true;
}

int Arpeggiator::getNextNote() {
  if (sortedNotes.empty())
    return -1;

  int numNotes = (int)sortedNotes.size();
  int root = sortedNotes[0];

  // --- MODE 1: STEP SEQUENCER (Arp Button OFF) ---
  if (!enabled) {
    int stepIdx = currentStep % 16;
    int seqVal = sequence[(size_t)stepIdx];

    if (seqVal == -1)
      return -1; // Rest

    // Scale Mapping (Minor-ish)
    constexpr int offsets[] = {0, 2, 3, 5, 7, 9, 10, 12};
    int offset = 0;
    if (seqVal >= 0 && seqVal < 8)
      offset = offsets[seqVal];

    return root + offset;
  }

  // --- MODE 2: ARPEGGIATOR (Arp Button ON) ---
  // Classic Arp patterns based on held notes

  int noteIdx = 0;

  switch (arpMode) {
  case 0: // UP
    noteIdx = currentStep % numNotes;
    break;

  case 1: // DOWN
    noteIdx = (numNotes - 1) - (currentStep % numNotes);
    break;

  case 2: // UP/DOWN
  {
    if (numNotes < 2) {
      noteIdx = 0;
    } else {
      int span = (numNotes * 2) - 2;
      int pos = currentStep % span;
      if (pos < numNotes)
        noteIdx = pos;
      else
        noteIdx = span - pos;
    }
  } break;

  case 3: // RANDOM
    // Simple pseudo-random based on step to be deterministic per loop?
    // Or true random? True random is livelier.
    noteIdx = juce::Random::getSystemRandom().nextInt(numNotes);
    break;
  }

  // Handle Octave Range
  // Simple logic: iterate notes, then next octave...
  // For now, let's keep it simple: mapped to sortedNotes directly + Octave
  // Wrap? Let's implement Octave Spanning: Virtual Notes = sortedNotes *
  // numOctaves.

  if (numOctaves > 1) {
    int totalVirtual = numNotes * numOctaves;
    // Re-calc index based on total virtual notes
    switch (arpMode) {
    case 0:
      noteIdx = currentStep % totalVirtual;
      break; // UP
    case 1:
      noteIdx = (totalVirtual - 1) - (currentStep % totalVirtual);
      break; // DOWN
      // ... simplify for now, just map noteIdx to Note + Octave
    }
  }

  // Correction for simple implementation above:
  // If we just stick to single octave for now to ensure it works.
  // Or:
  int octaveOffset = 0;
  if (numOctaves > 1) {
    int cycle = currentStep / numNotes;
    int oct = cycle % numOctaves;
    octaveOffset = oct * 12;
    // Wrap note index locally
    noteIdx = currentStep % numNotes;
  }

  if (noteIdx < 0)
    noteIdx = 0;
  if (noteIdx >= numNotes)
    noteIdx = 0;

  return sortedNotes[noteIdx] + octaveOffset;
}

double Arpeggiator::getSamplesPerStep(juce::AudioPlayHead *playHead) {
  double bpm = 120.0;
  if (playHead) {
    if (auto pos = playHead->getPosition()) {
      if (pos->getBpm().hasValue())
        bpm = *pos->getBpm();
    }
  }

  // Safety Clamp
  if (bpm < 20.0)
    bpm = 120.0;

  // RateDiv: 0=1/4, 1=1/8...
  double quarterNoteSamples = (60.0 / bpm) * currentSampleRate;

  if (rateDiv <= 0.1f)
    return quarterNoteSamples; // 1/4
  if (rateDiv <= 0.4f)
    return quarterNoteSamples / 2.0; // 1/8
  if (rateDiv <= 0.7f)
    return quarterNoteSamples / 4.0; // 1/16
  return quarterNoteSamples / 8.0;   // 1/32
}

void Arpeggiator::process(juce::MidiBuffer &midiMessages, int numSamples,
                          juce::AudioPlayHead *playHead) {

  // --- 1. Update State from Input ---
  for (const auto metadata : midiMessages) {
    auto msg = metadata.getMessage();
    if (msg.isNoteOn()) {
      handleNoteOn(msg.getNoteNumber(), msg.getVelocity());
    } else if (msg.isNoteOff()) {
      handleNoteOff(msg.getNoteNumber());
    } else if (msg.isAllNotesOff()) {
      reset();
    }
  }

  // --- 2. Check if we should generate anything ---
  // New Logic: If Disabled AND Grid is Empty -> Passthrough (Do nothing)
  // If Disabled BUT Grid has steps -> Run Sequencer Logic
  // If Enabled -> Run Arp Logic

  if (!enabled && isGridEmpty()) {
    // Standard Passthrough of events handled by flushing active notes logic
    // below? No, if we return here, we might leave active notes hanging if we
    // don't process offs. "activeNotes" tracks generated notes. Input notes
    // passed through? Wait, typical Arp replaces input. If Bypass: we should
    // just let input through? Current architecture checks
    // "MidiProcessor::process":
    // 1. Chords -> 2. Arp.
    // If Arp Bypassed, Chords output should pass.

    // BUT we must flush any *internally generated* notes from previous arp
    // state.
    for (auto it = activeNotes.begin(); it != activeNotes.end();) {
      if (it->samplesRemaining < numSamples) {
        midiMessages.addEvent(juce::MidiMessage::noteOff(1, it->noteNumber),
                              it->samplesRemaining);
        it = activeNotes.erase(it);
      } else {
        it->samplesRemaining -= numSamples;
        ++it;
      }
    }
    // And logic to pass original notes?
    // The loop at start "Update State" consumes messages? No, just reads.
    // The "3. Generate Sequence" block copies non-notes.
    // If we are in passthrough, we should normally just return?
    // MidiProcessor passes `midiMessages`. We just modified `arp` state.
    // If we do nothing, `midiMessages` stays as is (Chord Output).
    // PERFECT.
    return;
  }

  // --- 3. Generate Sequence (Arp or Seq) ---
  juce::MidiBuffer processedMidi;

  // Pass through non-note events (CC, etc)
  for (const auto metadata : midiMessages) {
    auto msg = metadata.getMessage();
    // Block original notes (we are replacing them with Arp/Seq notes)
    if (!msg.isNoteOn() && !msg.isNoteOff())
      processedMidi.addEvent(msg, metadata.samplePosition);
  }

  // Handle active note-offs
  for (auto it = activeNotes.begin(); it != activeNotes.end();) {
    if (it->samplesRemaining < numSamples) {
      processedMidi.addEvent(juce::MidiMessage::noteOff(1, it->noteNumber),
                             it->samplesRemaining);
      it = activeNotes.erase(it);
    } else {
      it->samplesRemaining -= numSamples;
      ++it;
    }
  }

  // Arp Logic
  if (sortedNotes.empty()) {
    midiMessages.swapWith(processedMidi);
    return;
  }

  double samplesPerStep = getSamplesPerStep(playHead);
  if (samplesPerStep < 100.0)
    samplesPerStep = 100.0;

  int samplesRemaining = numSamples;
  int currentSamplePos = 0;

  while (samplesRemaining > 0) {
    if (noteTime >= samplesPerStep) {
      noteTime -= samplesPerStep;

      int noteToPlay = getNextNote();

      if (noteToPlay > 0) {
        processedMidi.addEvent(
            juce::MidiMessage::noteOn(1, noteToPlay, (juce::uint8)100),
            currentSamplePos);

        int gateSamples = static_cast<int>(samplesPerStep * gateLength);

        ActiveNote an;
        an.noteNumber = noteToPlay;
        // relative to current block start: currentSamplePos + gateSamples
        // stored as remaining from NOW? No, remaining from block start usually?
        // Logic above uses `samplesRemaining` against `numSamples`.
        // Let's stick to: activeNotes stores samples relative to *current block
        // start*? No, typically stores "samples *remaining* until off".

        if (gateSamples < samplesRemaining) {
          // Ends within this block
          processedMidi.addEvent(juce::MidiMessage::noteOff(1, noteToPlay),
                                 currentSamplePos + gateSamples);
        } else {
          // Carries over
          an.samplesRemaining =
              gateSamples - (samplesRemaining); // wait, math check.
          // actually: an.samplesRemaining = (currentSamplePos + gateSamples) -
          // numSamples? No, simply: gateSamples is duration. allocated:
          // samplesRemaining (in this block). remaining = gateSamples -
          // samplesRemaining (left in block AFTER this pos) WAIT.

          // Correct math:
          // Time until end of block = numSamples - currentSamplePos.
          // If gate > TimeUntilEnd, then remainder = gate - TimeUntilEnd.
          an.samplesRemaining = gateSamples - (numSamples - currentSamplePos);
          activeNotes.push_back(an);
        }
      }
      currentStep++;
    }

    // Advance time
    int amount = std::min(samplesRemaining, 32); // Process in chunks
    // Optimization: advance to next event to avoid small steps?
    // For now, chunking is safe.

    // Check if we approach samplesPerStep
    double dist = samplesPerStep - noteTime;
    if (dist < 32.0 && dist > 0.0) {
      amount = (int)std::ceil(dist);
    }
    if (amount > samplesRemaining)
      amount = samplesRemaining;
    if (amount < 1)
      amount = 1;

    noteTime += amount;
    samplesRemaining -= amount;
    currentSamplePos += amount;
  }

  midiMessages.swapWith(processedMidi);
}

//==============================================================================
// ChordEngine
//==============================================================================

ChordEngine::ChordEngine() {}

void ChordEngine::setParameters(int mode, int keys, bool hold) {
  chordMode = mode;
  bool wasHolding = holdEnabled;
  holdEnabled = hold;

  // If we turned OFF hold, kill all stuck notes
  if (wasHolding && !holdEnabled) {
    shouldFlushNotes = true;
  }
}

void ChordEngine::process(juce::MidiBuffer &midiMessages) {
  juce::MidiBuffer processedBuf;

  // 1. Flush Stuck Notes if Hold was just disabled
  if (shouldFlushNotes) {
    for (int note : heldNotes) {
      processedBuf.addEvent(juce::MidiMessage::noteOff(1, note), 0);
    }
    heldNotes.clear();
    shouldFlushNotes = false;
  }

  // Interpret "Chord Hold" button as "Enable Chords" (User Request)
  // If button is ON (holdEnabled) but Mode is OFF, force Mode to MINOR (2)
  int effectiveMode = chordMode;
  if (holdEnabled && effectiveMode == 0) {
    effectiveMode = 2; // Default to Minor
  }

  if (effectiveMode == 0) {
    return;
  }

  // Clear original output buffer (we rebuild it)
  // Actually, standard practice for these processors: read from 'midiMessages',
  // write to 'processedBuf', then swap. BUT if we 'return', 'midiMessages'
  // keeps original content (Passthrough). So for Mode=0 returns, it's
  // Passthrough.

  for (const auto metadata : midiMessages) {
    auto msg = metadata.getMessage();

    if (msg.isNoteOn() || msg.isNoteOff()) {
      int root = msg.getNoteNumber();
      int vel = msg.getVelocity();
      bool isOn = msg.isNoteOn();

      // HOLD LOGIC:
      // If NoteOff AND Hold is ON -> Ignore (don't add to processedBuf)
      // But we must remember it to kill it later.
      if (!isOn && holdEnabled) {
        // Ignore Note Off
        continue;
      }

      auto addEvent = [&](int note) {
        if (isOn) {
          processedBuf.addEvent(
              juce::MidiMessage::noteOn(1, note, (juce::uint8)vel),
              metadata.samplePosition);
          if (holdEnabled)
            heldNotes.insert(note);
        } else {
          processedBuf.addEvent(juce::MidiMessage::noteOff(1, note),
                                metadata.samplePosition);
          if (heldNotes.find(note) != heldNotes.end()) {
            heldNotes.erase(note);
          }
        }
      };

      addEvent(root);

      switch (effectiveMode) {
      case 1: // Major
        addEvent(root + 4);
        addEvent(root + 7);
        break;
      case 2: // Minor
        addEvent(root + 3);
        addEvent(root + 7);
        break;
      case 3: // 7th
        addEvent(root + 4);
        addEvent(root + 7);
        addEvent(root + 10);
        break;
      case 4: // 9th
        addEvent(root + 4);
        addEvent(root + 7);
        addEvent(root + 14);
        break;
      default:
        break;
      }

    } else {
      processedBuf.addEvent(msg, metadata.samplePosition);
    }
  }

  midiMessages.swapWith(processedBuf);
}

//==============================================================================
// MidiProcessor
//==============================================================================

MidiProcessor::MidiProcessor() {}

void MidiProcessor::prepare(double sampleRate) {
  currentSampleRate = sampleRate;
  arp.prepare(sampleRate);
}

void MidiProcessor::reset() { arp.reset(); }

void MidiProcessor::process(juce::MidiBuffer &midiMessages, int numSamples,
                            juce::AudioPlayHead *playHead) {
  // 1. Chords First
  chordEngine.process(midiMessages);

  // 2. Arp Second
  arp.process(midiMessages, numSamples, playHead);
}
