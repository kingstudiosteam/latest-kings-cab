#include "PluginProcessor.h"
#include "PluginEditor.h"

CrossFXEnhancedAudioProcessor::CrossFXEnhancedAudioProcessor()
  : juce::AudioProcessor(BusesProperties()
      .withInput("Main Input", juce::AudioChannelSet::stereo(), true)
      .withInput("Sidechain", juce::AudioChannelSet::stereo(), true)
      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    valueTreeState(*this, nullptr, "PARAMS", createParameterLayout())
{
  blendParam = valueTreeState.getRawParameterValue("blend");
  gainAParam = valueTreeState.getRawParameterValue("gainA");
  gainBParam = valueTreeState.getRawParameterValue("gainB");
  fadeModeParam = valueTreeState.getRawParameterValue("fadeMode");
  
  // Clipper/Limiter parameters
  clipperTypeParam = valueTreeState.getRawParameterValue("clipperType");
  limiterTypeParam = valueTreeState.getRawParameterValue("limiterType");
  thresholdParam = valueTreeState.getRawParameterValue("threshold");
  ceilingParam = valueTreeState.getRawParameterValue("ceiling");
  attackParam = valueTreeState.getRawParameterValue("attack");
  releaseParam = valueTreeState.getRawParameterValue("release");
  ratioParam = valueTreeState.getRawParameterValue("ratio");
  kneeParam = valueTreeState.getRawParameterValue("knee");

  startTimerHz(30);
}

bool CrossFXEnhancedAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
  const auto& in = layouts.getMainInputChannelSet();
  const auto& out = layouts.getMainOutputChannelSet();
  if (in != out) return false;
  return (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo());
}

void CrossFXEnhancedAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
  // Prepare clipper/limiters
  juce::dsp::ProcessSpec spec;
  spec.sampleRate = sampleRate;
  spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
  spec.numChannels = 1;
  
  clipperLimiterA.prepare(spec);
  clipperLimiterB.prepare(spec);
}

void CrossFXEnhancedAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
  juce::ScopedNoDenormals noDenormals;

  auto aIn = getBusBuffer(buffer, true, 0);
  auto bIn = getBusBuffer(buffer, true, 1);
  auto out = getBusBuffer(buffer, false, 0);

  const int numSamples = buffer.getNumSamples();
  const int numChannels = juce::jmin(out.getNumChannels(), aIn.getNumChannels());
  // Rollback: no ring capture

  // Run auto-align when requested
  // Rollback: no alignment processing

  // Apply user invert option
  const bool invA = false; // rollback
  const bool invB = false; // rollback

  const float t = juce::jlimit(0.0f, 1.0f, blendParam->load());
  const int mode = static_cast<int>(fadeModeParam->load() + 0.5f);
  float wA = 1.0f - t;
  float wB = t;
  if (mode == 1) // Smooth (S-curve)
  {
    const float s = t * t * (3.0f - 2.0f * t);
    wA = 1.0f - s;
    wB = s;
  }
  else if (mode == 2) // EqualPower
  {
    const float theta = t * juce::MathConstants<float>::halfPi;
    wA = std::cos(theta);
    wB = std::sin(theta);
  }

  const float preA = juce::Decibels::decibelsToGain(gainAParam->load());
  const float preB = juce::Decibels::decibelsToGain(gainBParam->load());
  
  // Update clipper/limiter parameters
  updateClipperLimiterParameters();

  float peakA = 0.0f, peakB = 0.0f;           // pre-gain (unused for meters)
  float peakAGain = 0.0f, peakBGain = 0.0f;   // post-gain for meters
  double sumSqA = 0.0, sumSqB = 0.0;

  for (int ch = 0; ch < numChannels; ++ch)
  {
    const float* inA = aIn.getReadPointer(ch);
    const float* inB = bIn.getNumChannels() > ch ? bIn.getReadPointer(ch) : nullptr;
    float* outCh = out.getWritePointer(ch);

    for (int i = 0; i < numSamples; ++i)
    {
      float a = inA[i];
      float bS = inB ? inB[i] : 0.0f;
      if (invA) a = -a;
      if (invB) bS = -bS;
      const float aG = preA * wA * a;
      const float bG = preB * wB * bS;
      // Meter should reflect post-gain per input regardless of blend
      const float aMeter = preA * a;
      const float bMeter = preB * bS;
      
      // Apply clipper/limiter to individual channels before blending
      float aClipped = aG;
      float bClipped = bG;
      
      if (clipperTypeParam->load() > 0.0f || limiterTypeParam->load() > 0.0f)
      {
        clipperLimiterA.process(&aClipped, 1);
        clipperLimiterB.process(&bClipped, 1);
      }
      
      outCh[i] = aClipped + bClipped;

      const float absA = std::abs(a);
      const float absBv = std::abs(bS);
      if (absA > peakA) peakA = absA;
      if (absBv > peakB) peakB = absBv;
      const float absAG = std::abs(aMeter);
      const float absBG = std::abs(bMeter);
      if (absAG > peakAGain) peakAGain = absAG;
      if (absBG > peakBGain) peakBGain = absBG;
      sumSqA += (double)aMeter * (double)aMeter;
      sumSqB += (double)bMeter * (double)bMeter;
    }
  }

  // Slower peak hold with gentle fall
  const float fall = 0.995f; // gentle fall per timer tick (~30 Hz)
  const int holdTicksMax = 75; // ~2.5 sec hold at 30 Hz

  float curA = inputAPeak.load();
  float curB = inputBPeak.load();
  if (peakAGain > curA) { inputAPeak.store(peakAGain); peakAHoldTicks.store(holdTicksMax); }
  else {
    int h = peakAHoldTicks.load();
    if (h > 0) peakAHoldTicks.store(h - 1);
    else inputAPeak.store(curA * fall);
  }
  if (peakBGain > curB) { inputBPeak.store(peakBGain); peakBHoldTicks.store(holdTicksMax); }
  else {
    int h = peakBHoldTicks.load();
    if (h > 0) peakBHoldTicks.store(h - 1);
    else inputBPeak.store(curB * fall);
  }

  // RMS rolling (per block)
  const float rmsA = std::sqrt((float)(sumSqA / juce::jmax(1, numSamples)));
  const float rmsB = std::sqrt((float)(sumSqB / juce::jmax(1, numSamples)));
  lastRmsA.store(rmsA);
  lastRmsB.store(rmsB);
  // EWMA smoothing (~1s at 30 Hz): alpha ~ 1 - exp(-dt/tau)
  const float alpha = 0.05f;
  ewmaRmsA.store(ewmaRmsA.load() * (1.0f - alpha) + rmsA * alpha);
  ewmaRmsB.store(ewmaRmsB.load() * (1.0f - alpha) + rmsB * alpha);
  if (peakA >= 0.999f) inputAClip.store(20); else inputAClip.store(std::max(0, inputAClip.load() - 1));
  if (peakB >= 0.999f) inputBClip.store(20); else inputBClip.store(std::max(0, inputBClip.load() - 1));
}

void CrossFXEnhancedAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
  if (auto state = valueTreeState.copyState(); auto xml = state.createXml())
    copyXmlToBinary(*xml, destData);
}

void CrossFXEnhancedAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
  if (auto xml = getXmlFromBinary(data, sizeInBytes))
    valueTreeState.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* CrossFXEnhancedAudioProcessor::createEditor()
{
  return new CrossFXAudioProcessorEditor(*this);
}

CrossFXEnhancedAudioProcessor::APVTS::ParameterLayout CrossFXEnhancedAudioProcessor::createParameterLayout()
{
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

  // Rollback: no alignment parameters in this safe point

  // Core blend controls next
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID{"blend", 1}, "Blend",
    juce::NormalisableRange<float>{ 0.0f, 1.0f, 0.0001f }, 0.0f));
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID{"gainA", 1}, "Gain A",
    juce::NormalisableRange<float>{ -24.0f, 24.0f, 0.01f }, 0.0f,
    juce::AudioParameterFloatAttributes().withLabel("dB")));
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID{"gainB", 1}, "Gain B",
    juce::NormalisableRange<float>{ -24.0f, 24.0f, 0.01f }, 0.0f,
    juce::AudioParameterFloatAttributes().withLabel("dB")));
  params.push_back(std::make_unique<juce::AudioParameterChoice>(
    juce::ParameterID{"fadeMode", 1}, "Fade",
    juce::StringArray{ "Linear", "Smooth", "EqualPower" }, 2));
  
  // Clipper/Limiter parameters
  params.push_back(std::make_unique<juce::AudioParameterChoice>(
    juce::ParameterID{"clipperType", 1}, "Clipper",
    juce::StringArray{ "None", "Soft Tanh", "Hard Clip", "Cubic", "Hermite", "Foldback" }, 0));
  
  params.push_back(std::make_unique<juce::AudioParameterChoice>(
    juce::ParameterID{"limiterType", 1}, "Limiter",
    juce::StringArray{ "None", "Feedback", "Feedforward", "Look Ahead" }, 0));
  
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID{"threshold", 1}, "Threshold",
    juce::NormalisableRange<float>{ -60.0f, 0.0f, 0.1f }, -12.0f,
    juce::AudioParameterFloatAttributes().withLabel("dB")));
  
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID{"ceiling", 1}, "Ceiling",
    juce::NormalisableRange<float>{ -60.0f, 0.0f, 0.1f }, -0.1f,
    juce::AudioParameterFloatAttributes().withLabel("dB")));
  
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID{"attack", 1}, "Attack",
    juce::NormalisableRange<float>{ 0.1f, 100.0f, 0.1f }, 1.0f,
    juce::AudioParameterFloatAttributes().withLabel("ms")));
  
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID{"release", 1}, "Release",
    juce::NormalisableRange<float>{ 1.0f, 1000.0f, 1.0f }, 50.0f,
    juce::AudioParameterFloatAttributes().withLabel("ms")));
  
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID{"ratio", 1}, "Ratio",
    juce::NormalisableRange<float>{ 1.0f, 20.0f, 0.1f }, 4.0f,
    juce::AudioParameterFloatAttributes().withLabel(":1")));
  
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID{"knee", 1}, "Knee",
    juce::NormalisableRange<float>{ 0.0f, 24.0f, 0.1f }, 6.0f,
    juce::AudioParameterFloatAttributes().withLabel("dB")));

  return { params.begin(), params.end() };
}

void CrossFXEnhancedAudioProcessor::updateClipperLimiterParameters()
{
  // Update clipper type
  const int clipperType = static_cast<int>(clipperTypeParam->load() + 0.5f);
  clipperLimiterA.setClipperType(static_cast<ClipperLimiter::ClipperType>(clipperType));
  clipperLimiterB.setClipperType(static_cast<ClipperLimiter::ClipperType>(clipperType));
  
  // Update limiter type
  const int limiterType = static_cast<int>(limiterTypeParam->load() + 0.5f);
  clipperLimiterA.setLimiterType(static_cast<ClipperLimiter::LimiterType>(limiterType));
  clipperLimiterB.setLimiterType(static_cast<ClipperLimiter::LimiterType>(limiterType));
  
  // Update other parameters
  clipperLimiterA.setThreshold(thresholdParam->load());
  clipperLimiterB.setThreshold(thresholdParam->load());
  clipperLimiterA.setCeiling(ceilingParam->load());
  clipperLimiterB.setCeiling(ceilingParam->load());
  clipperLimiterA.setAttack(attackParam->load());
  clipperLimiterB.setAttack(attackParam->load());
  clipperLimiterA.setRelease(releaseParam->load());
  clipperLimiterB.setRelease(releaseParam->load());
  clipperLimiterA.setRatio(ratioParam->load());
  clipperLimiterB.setRatio(ratioParam->load());
  clipperLimiterA.setKnee(kneeParam->load());
  clipperLimiterB.setKnee(kneeParam->load());
  
  // Update meters
  clipperAGainReduction.store(clipperLimiterA.getGainReduction());
  clipperBGainReduction.store(clipperLimiterB.getGainReduction());
  clipperAInputLevel.store(clipperLimiterA.getInputLevel());
  clipperBInputLevel.store(clipperLimiterB.getInputLevel());
  clipperAOutputLevel.store(clipperLimiterA.getOutputLevel());
  clipperBOutputLevel.store(clipperLimiterB.getOutputLevel());
}

// JUCE factory
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
  return new CrossFXEnhancedAudioProcessor();
}

void CrossFXEnhancedAudioProcessor::timerCallback()
{
  // Reduce meter values when no audio is present, so meters fall back to zero
  decayMeters(0.9f);
}

void CrossFXEnhancedAudioProcessor::decayMeters(float factor)
{
  inputAPeak.store(inputAPeak.load() * factor);
  inputBPeak.store(inputBPeak.load() * factor);
  inputAClip.store(std::max(0, inputAClip.load() - 1));
  inputBClip.store(std::max(0, inputBClip.load() - 1));
}

void CrossFXEnhancedAudioProcessor::autoGainMatchToEqual()
{
  // Peak-match: adjust Gain B so peak B matches peak A (post-gain)
  const float peakA = inputAPeak.load();
  const float peakB = inputBPeak.load();
  if (peakA <= 0.0f || peakB <= 0.0f)
    return;

  const float deltaDb = juce::Decibels::gainToDecibels(peakA) - juce::Decibels::gainToDecibels(peakB);
  const float currentDb = gainBParam->load();
  // Apply full correction in one press, clamped to slider bounds
  const float newDb = juce::jlimit(-24.0f, 24.0f, currentDb + deltaDb);
  valueTreeState.getParameter("gainB")->beginChangeGesture();
  valueTreeState.getParameter("gainB")->setValueNotifyingHost((newDb + 24.0f) / 48.0f);
  valueTreeState.getParameter("gainB")->endChangeGesture();
}

// Rollback: no alignment helpers


