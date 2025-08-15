#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "ClipperLimiter.h"
// Rollback: no DSP delay lines needed

class CrossFXEnhancedAudioProcessor : public juce::AudioProcessor, public juce::Timer
{
public:
  CrossFXEnhancedAudioProcessor();
  ~CrossFXEnhancedAudioProcessor() override = default;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override {}
  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override { return true; }

  const juce::String getName() const override { return JucePlugin_Name; }
  bool acceptsMidi() const override { return false; }
  bool producesMidi() const override { return false; }
  double getTailLengthSeconds() const override { return 0.0; }

  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return {}; }
  void changeProgramName(int, const juce::String&) override {}

  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;

  using APVTS = juce::AudioProcessorValueTreeState;
  APVTS& getValueTreeState() { return valueTreeState; }

  static APVTS::ParameterLayout createParameterLayout();

  // Meter accessors
  float getInputAPeak() const { return inputAPeak.load(); }
  float getInputBPeak() const { return inputBPeak.load(); }
  bool getInputAClipped() const { return inputAClip.load() > 0; }
  bool getInputBClipped() const { return inputBClip.load() > 0; }
  float getInputAdB() const { return juce::Decibels::gainToDecibels(inputAPeak.load(), -100.0f); }
  float getInputBdB() const { return juce::Decibels::gainToDecibels(inputBPeak.load(), -100.0f); }
  void autoGainMatchToEqual();

  void decayMeters(float factor);
  
  void updateClipperLimiterParameters();

  void timerCallback() override;

private:
  APVTS valueTreeState;
  std::atomic<float>* blendParam { nullptr }; // A/B
  std::atomic<float>* gainAParam { nullptr };
  std::atomic<float>* gainBParam { nullptr };
  std::atomic<float>* fadeModeParam { nullptr }; // 0=Linear,1=Smooth,2=EqualPower
  
  // Clipper/Limiter parameters
  std::atomic<float>* clipperTypeParam { nullptr };
  std::atomic<float>* limiterTypeParam { nullptr };
  std::atomic<float>* thresholdParam { nullptr };
  std::atomic<float>* ceilingParam { nullptr };
  std::atomic<float>* attackParam { nullptr };
  std::atomic<float>* releaseParam { nullptr };
  std::atomic<float>* ratioParam { nullptr };
  std::atomic<float>* kneeParam { nullptr };

  std::atomic<float> inputAPeak { 0.0f };
  std::atomic<float> inputBPeak { 0.0f };
  std::atomic<int> inputAClip { 0 };
  std::atomic<int> inputBClip { 0 };
  std::atomic<int> peakAHoldTicks { 0 };
  std::atomic<int> peakBHoldTicks { 0 };
  std::atomic<float> lastRmsA { 0.0f };
  std::atomic<float> lastRmsB { 0.0f };
  std::atomic<float> ewmaRmsA { 0.0f };
  std::atomic<float> ewmaRmsB { 0.0f };

  // Clipper/Limiter instances
  ClipperLimiter clipperLimiterA;
  ClipperLimiter clipperLimiterB;
  
  // Clipper/Limiter meters
  std::atomic<float> clipperAGainReduction { 0.0f };
  std::atomic<float> clipperBGainReduction { 0.0f };
  std::atomic<float> clipperAInputLevel { 0.0f };
  std::atomic<float> clipperBInputLevel { 0.0f };
  std::atomic<float> clipperAOutputLevel { 0.0f };
  std::atomic<float> clipperBOutputLevel { 0.0f };

  // Rollback: no phase align state
};


