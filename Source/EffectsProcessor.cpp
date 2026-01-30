#include "EffectsProcessor.h"

EffectsProcessor::EffectsProcessor() {
  // Initialize Waveshaper with tanh transfer function
  distortion.functionToUse = [](float x) { return std::tanh(x); };

  // Delay setup
  delayLine.setMaximumDelayInSamples(44100 * 2);
}

EffectsProcessor::~EffectsProcessor() {}

void EffectsProcessor::prepare(juce::dsp::ProcessSpec &spec) {
  currentSampleRate = spec.sampleRate;

  // Prepare Distortion
  distortion.prepare(spec);
  distDriveParam.reset(currentSampleRate, 0.05); // 50ms ramp
  distMixParam.reset(currentSampleRate, 0.05);

  // Prepare Transient Shaper
  transientShaper.prepare(spec);

  // Prepare Delay
  delayLine.prepare(spec);
  // Important: Set correct max delay based on sample rate
  delayLine.setMaximumDelayInSamples(
      static_cast<int>(spec.sampleRate * maxDelayTime));
  delayLine.reset();

  delayTimeParam.reset(currentSampleRate,
                       0.5); // Slower ramp for delay time to avoid pitch jumps?
                             // Actually fast ramp + interpolation is better for
                             // "swoop". 0.05 is standard.
  delayTimeParam.reset(currentSampleRate, 0.05);
  delayFeedbackParam.reset(currentSampleRate, 0.05);
  delayMixParam.reset(currentSampleRate, 0.05);

  // Prepare Reverb
  reverb.prepare(spec);
  reverb.reset();
  reverbMixParam.reset(currentSampleRate, 0.05);

  // Reserve ramp buffer
  rampBuffer.reserve(spec.maximumBlockSize);
}

void EffectsProcessor::reset() {
  distortion.reset();
  transientShaper.reset();
  delayLine.reset();
  reverb.reset();

  // Reset smoothers to target ?? No, usually just keep current.
}

void EffectsProcessor::updateParameters(float distDrive, float distMix,
                                        float delayTime, float delayFeedback,
                                        float delayMix, float reverbSize,
                                        float reverbDamping, float reverbMix,
                                        float biteAmount) {
  distDriveParam.setTargetValue(distDrive);
  distMixParam.setTargetValue(distMix);
  transientShaper.setAmount(biteAmount); // Shaper handles its own smoothing

  delayTimeParam.setTargetValue(delayTime);
  delayFeedbackParam.setTargetValue(delayFeedback);
  delayMixParam.setTargetValue(delayMix);

  // Map Reverb params
  reverbParams.roomSize = reverbSize;
  reverbParams.damping = reverbDamping;

  // Revert to internal mixing to ensure Dry/Wet balance works without temp
  // buffer
  reverbParams.wetLevel = reverbMix;
  reverbParams.dryLevel = 1.0f - reverbMix;

  reverbParams.width = 1.0f;
  reverbParams.freezeMode = 0.0f;

  reverb.setParameters(reverbParams);
  reverbMixParam.setTargetValue(
      reverbMix); // Keep for the on/off check in processReverb
}

void EffectsProcessor::process(juce::AudioBuffer<float> &buffer) {
  juce::ScopedNoDenormals noDenormals;

  for (auto effect : chainOrder) {
    switch (effect) {
    case EffectType::Distortion:
      processDistortion(buffer);
      break;
    case EffectType::TransientShaper:
      processTransientShaper(buffer);
      break;
    case EffectType::Delay:
      processDelay(buffer);
      break;
    case EffectType::Reverb:
      processReverb(buffer);
      break;
    }
  }
}

void EffectsProcessor::processDistortion(juce::AudioBuffer<float> &buffer) {
  auto totalNumInputChannels = buffer.getNumChannels();
  auto numSamples = buffer.getNumSamples();

  auto *ch0 = buffer.getWritePointer(0);
  auto *ch1 = (totalNumInputChannels > 1) ? buffer.getWritePointer(1) : nullptr;

  for (int i = 0; i < numSamples; ++i) {
    float drive = distDriveParam.getNextValue();
    float mix = distMixParam.getNextValue();

    float gain = 1.0f + (drive * 49.0f);

    float dry0 = ch0[i];
    float wet0 = std::tanh(dry0 * gain);
    ch0[i] = (dry0 * (1.0f - mix)) + (wet0 * mix);

    if (ch1) {
      float dry1 = ch1[i];
      float wet1 = std::tanh(dry1 * gain);
      ch1[i] = (dry1 * (1.0f - mix)) + (wet1 * mix);
    }
  }
}

void EffectsProcessor::processTransientShaper(
    juce::AudioBuffer<float> &buffer) {
  transientShaper.process(buffer);
}

void EffectsProcessor::processDelay(juce::AudioBuffer<float> &buffer) {
  auto totalNumInputChannels = buffer.getNumChannels();
  auto numSamples = buffer.getNumSamples();

  auto *ch0 = buffer.getWritePointer(0);
  auto *ch1 = (totalNumInputChannels > 1) ? buffer.getWritePointer(1) : nullptr;

  for (int i = 0; i < numSamples; ++i) {
    float time = delayTimeParam.getNextValue();
    float fdbk = delayFeedbackParam.getNextValue();
    float mix = delayMixParam.getNextValue();

    float delaySamples = time * (float)currentSampleRate;
    delaySamples = juce::jlimit(1.0f, (float)(maxDelayTime * currentSampleRate),
                                delaySamples);

    delayLine.setDelay(delaySamples);

    {
      float input = ch0[i];
      float delayed = delayLine.popSample(0, -1.0f);
      float fbSignal = input + (delayed * fdbk);
      delayLine.pushSample(0, fbSignal);
      ch0[i] = input + (delayed * mix);
    }

    if (ch1) {
      float input = ch1[i];
      float delayed = delayLine.popSample(1, -1.0f);
      float fbSignal = input + (delayed * fdbk);
      delayLine.pushSample(1, fbSignal);
      ch1[i] = input + (delayed * mix);
    }
  }
}

void EffectsProcessor::processReverb(juce::AudioBuffer<float> &buffer) {
  // Use TargetValue because we are using block-based mixing via setParameters,
  // so the Smoother isn't technically advanced per sample, but Target holds
  // current setting.
  if (reverbMixParam.getTargetValue() > 0.0f) {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);
  }
}
