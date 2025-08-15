#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "LookAndFeel.h"

class CrossFXEnhancedAudioProcessor;

class CrossFXAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
  explicit CrossFXAudioProcessorEditor(CrossFXEnhancedAudioProcessor& processor);
  ~CrossFXAudioProcessorEditor() override = default;

  void paint(juce::Graphics&) override;
  void resized() override;
  void timerCallback() override;

private:
  CrossFXEnhancedAudioProcessor& audioProcessor;
  MultiBlenderLookAndFeel lnF;
  juce::Slider blendSlider;
  juce::Slider gainASlider;
  juce::Slider gainBSlider;
  juce::ComboBox fadeModeBox;
  juce::Label dbALabel;
  juce::Label dbBLabel;
  juce::TextButton autoGainButton { "Auto Gain" };
  
  // Clipper/Limiter UI
  juce::ComboBox clipperTypeBox;
  juce::ComboBox limiterTypeBox;
  juce::Slider thresholdSlider;
  juce::Slider ceilingSlider;
  juce::Slider attackSlider;
  juce::Slider releaseSlider;
  juce::Slider ratioSlider;
  juce::Slider kneeSlider;
  juce::Label thresholdLabel;
  juce::Label ceilingLabel;
  juce::Label attackLabel;
  juce::Label releaseLabel;
  juce::Label ratioLabel;
  juce::Label kneeLabel;
  
  // Clipper/Limiter meters
  juce::Label grALabel;
  juce::Label grBLabel;
  juce::Label inputALabel;
  juce::Label inputBLabel;
  juce::Label outputALabel;
  juce::Label outputBLabel;

  juce::Image logoImage;
  

  using APVTS = juce::AudioProcessorValueTreeState;
  std::unique_ptr<juce::SliderParameterAttachment> blendAttachment;
  std::unique_ptr<juce::SliderParameterAttachment> gainAAttachment;
  std::unique_ptr<juce::SliderParameterAttachment> gainBAttachment;
  std::unique_ptr<juce::ComboBoxParameterAttachment> fadeModeAttachment;
  
  // Clipper/Limiter attachments
  std::unique_ptr<juce::ComboBoxParameterAttachment> clipperTypeAttachment;
  std::unique_ptr<juce::ComboBoxParameterAttachment> limiterTypeAttachment;
  std::unique_ptr<juce::SliderParameterAttachment> thresholdAttachment;
  std::unique_ptr<juce::SliderParameterAttachment> ceilingAttachment;
  std::unique_ptr<juce::SliderParameterAttachment> attackAttachment;
  std::unique_ptr<juce::SliderParameterAttachment> releaseAttachment;
  std::unique_ptr<juce::SliderParameterAttachment> ratioAttachment;
  std::unique_ptr<juce::SliderParameterAttachment> kneeAttachment;
  

  void setupClipperLimiterUI();
  void drawMeter(juce::Graphics& g, juce::Rectangle<int> bounds, float peak, juce::Colour fill);
  void drawMeterTicks(juce::Graphics& g, juce::Rectangle<int> bounds);
};


