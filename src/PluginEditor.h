#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "LookAndFeel.h"

class CrossFXAudioProcessor;

class CrossFXAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
  explicit CrossFXAudioProcessorEditor(CrossFXAudioProcessor& processor);
  ~CrossFXAudioProcessorEditor() override = default;

  void paint(juce::Graphics&) override;
  void resized() override;
  void timerCallback() override;

private:
  CrossFXAudioProcessor& audioProcessor;
  MultiBlenderLookAndFeel lnF;
  juce::Slider blendSlider;
  juce::Slider gainASlider;
  juce::Slider gainBSlider;
  juce::ComboBox fadeModeBox;
  juce::Label dbALabel;
  juce::Label dbBLabel;
  juce::TextButton autoGainButton { "Auto Gain" };
  // Rollback: no alignment UI

  juce::Image logoImage;
  

  using APVTS = juce::AudioProcessorValueTreeState;
  std::unique_ptr<juce::SliderParameterAttachment> blendAttachment;
  std::unique_ptr<juce::SliderParameterAttachment> gainAAttachment;
  std::unique_ptr<juce::SliderParameterAttachment> gainBAttachment;
  std::unique_ptr<juce::ComboBoxParameterAttachment> fadeModeAttachment;
  // Rollback: no alignment attachments
  

  void drawMeter(juce::Graphics& g, juce::Rectangle<int> bounds, float peak, juce::Colour fill);
  void drawMeterTicks(juce::Graphics& g, juce::Rectangle<int> bounds);
};


