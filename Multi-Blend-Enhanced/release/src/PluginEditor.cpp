#include "PluginEditor.h"
#include "PluginProcessor.h"
#include <BinaryData.h>

CrossFXAudioProcessorEditor::CrossFXAudioProcessorEditor(CrossFXEnhancedAudioProcessor& p)
  : juce::AudioProcessorEditor(&p), audioProcessor(p)
{
  setSize(800, 500);
  setLookAndFeel(&lnF);
  startTimerHz(30);

  // Load logo if present
  {
    juce::File logoFile(juce::File::getSpecialLocation(juce::File::userHomeDirectory)
                          .getChildFile("Dev/CrossFX/assets/multiblend_logo.png"));
    if (logoFile.existsAsFile())
    {
      auto stream = logoFile.createInputStream();
      if (stream != nullptr)
        logoImage = juce::PNGImageFormat::loadFrom(*stream);
    }
    if (! logoImage.isValid())
    {
      if (BinaryData::multiblend_logo_pngSize > 0)
        logoImage = juce::ImageFileFormat::loadFrom(BinaryData::multiblend_logo_png,
                                                    (size_t) BinaryData::multiblend_logo_pngSize);
    }
  }

  auto setupSlider = [](juce::Slider& s, juce::Slider::SliderStyle style)
  {
    s.setSliderStyle(style);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
  };

  setupSlider(blendSlider, juce::Slider::LinearHorizontal);
  gainASlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  gainASlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
  gainBSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  gainBSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
  // Set rotary parameters so 12 o'clock is unity (0 dB)
  const float mid = juce::MathConstants<float>::pi * 1.5f; // 270° => 12 o'clock in JUCE
  const float sweep = juce::MathConstants<float>::pi * 1.5f;     // 270° total sweep
  const float start = mid - sweep * 0.5f; // centered around up
  const float end   = mid + sweep * 0.5f;
  gainASlider.setRotaryParameters(start, end, true);
  gainBSlider.setRotaryParameters(start, end, true);

  addAndMakeVisible(blendSlider);
  addAndMakeVisible(gainASlider);
  addAndMakeVisible(gainBSlider);
  addAndMakeVisible(fadeModeBox);
  addAndMakeVisible(autoGainButton);
  addAndMakeVisible(dbALabel);
  addAndMakeVisible(dbBLabel);

  // Populate fade mode choices (ensure visible on all hosts)
  fadeModeBox.clear(juce::dontSendNotification);
  fadeModeBox.addItem("Linear", 1);
  fadeModeBox.addItem("Smooth", 2);
  fadeModeBox.addItem("EqualPower", 3);

  auto& vts = audioProcessor.getValueTreeState();
  blendAttachment = std::make_unique<juce::SliderParameterAttachment>(*vts.getParameter("blend"), blendSlider);
  gainAAttachment = std::make_unique<juce::SliderParameterAttachment>(*vts.getParameter("gainA"), gainASlider);
  gainBAttachment = std::make_unique<juce::SliderParameterAttachment>(*vts.getParameter("gainB"), gainBSlider);
  fadeModeAttachment  = std::make_unique<juce::ComboBoxParameterAttachment>(*vts.getParameter("fadeMode"), fadeModeBox);
  autoGainButton.onClick = [this]{ audioProcessor.autoGainMatchToEqual(); };
  dbALabel.setJustificationType(juce::Justification::centred);
  dbALabel.setColour(juce::Label::textColourId, juce::Colours::white);
  dbBLabel.setJustificationType(juce::Justification::centred);
  dbBLabel.setColour(juce::Label::textColourId, juce::Colours::white);

  // Setup Clipper/Limiter UI
  setupClipperLimiterUI();
}

void CrossFXAudioProcessorEditor::setupClipperLimiterUI()
{
  auto& vts = audioProcessor.getValueTreeState();
  
  // Setup clipper type combo box
  clipperTypeBox.clear(juce::dontSendNotification);
  clipperTypeBox.addItem("None", 1);
  clipperTypeBox.addItem("Soft Tanh", 2);
  clipperTypeBox.addItem("Hard Clip", 3);
  clipperTypeBox.addItem("Cubic", 4);
  clipperTypeBox.addItem("Hermite", 5);
  clipperTypeBox.addItem("Foldback", 6);
  
  // Setup limiter type combo box
  limiterTypeBox.clear(juce::dontSendNotification);
  limiterTypeBox.addItem("None", 1);
  limiterTypeBox.addItem("Feedback", 2);
  limiterTypeBox.addItem("Feedforward", 3);
  limiterTypeBox.addItem("Look Ahead", 4);
  
  // Setup sliders
  thresholdSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
  ceilingSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  ceilingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
  attackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
  releaseSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
  ratioSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  ratioSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
  kneeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  kneeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
  
  // Setup labels
  thresholdLabel.setText("Threshold", juce::dontSendNotification);
  thresholdLabel.setJustificationType(juce::Justification::centred);
  thresholdLabel.setColour(juce::Label::textColourId, juce::Colours::white);
  
  ceilingLabel.setText("Ceiling", juce::dontSendNotification);
  ceilingLabel.setJustificationType(juce::Justification::centred);
  ceilingLabel.setColour(juce::Label::textColourId, juce::Colours::white);
  
  attackLabel.setText("Attack", juce::dontSendNotification);
  attackLabel.setJustificationType(juce::Justification::centred);
  attackLabel.setColour(juce::Label::textColourId, juce::Colours::white);
  
  releaseLabel.setText("Release", juce::dontSendNotification);
  releaseLabel.setJustificationType(juce::Justification::centred);
  releaseLabel.setColour(juce::Label::textColourId, juce::Colours::white);
  
  ratioLabel.setText("Ratio", juce::dontSendNotification);
  ratioLabel.setJustificationType(juce::Justification::centred);
  ratioLabel.setColour(juce::Label::textColourId, juce::Colours::white);
  
  kneeLabel.setText("Knee", juce::dontSendNotification);
  kneeLabel.setJustificationType(juce::Justification::centred);
  kneeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
  
  // Setup meter labels
  grALabel.setText("GR A", juce::dontSendNotification);
  grALabel.setJustificationType(juce::Justification::centred);
  grALabel.setColour(juce::Label::textColourId, juce::Colours::yellow);
  
  grBLabel.setText("GR B", juce::dontSendNotification);
  grBLabel.setJustificationType(juce::Justification::centred);
  grBLabel.setColour(juce::Label::textColourId, juce::Colours::yellow);
  
  inputALabel.setText("In A", juce::dontSendNotification);
  inputALabel.setJustificationType(juce::Justification::centred);
  inputALabel.setColour(juce::Label::textColourId, juce::Colours::lightblue);
  
  inputBLabel.setText("In B", juce::dontSendNotification);
  inputBLabel.setJustificationType(juce::Justification::centred);
  inputBLabel.setColour(juce::Label::textColourId, juce::Colours::lightblue);
  
  outputALabel.setText("Out A", juce::dontSendNotification);
  outputALabel.setJustificationType(juce::Justification::centred);
  outputALabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
  
  outputBLabel.setText("Out B", juce::dontSendNotification);
  outputBLabel.setJustificationType(juce::Justification::centred);
  outputBLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
  
  // Add components to UI
  addAndMakeVisible(clipperTypeBox);
  addAndMakeVisible(limiterTypeBox);
  addAndMakeVisible(thresholdSlider);
  addAndMakeVisible(ceilingSlider);
  addAndMakeVisible(attackSlider);
  addAndMakeVisible(releaseSlider);
  addAndMakeVisible(ratioSlider);
  addAndMakeVisible(kneeSlider);
  addAndMakeVisible(thresholdLabel);
  addAndMakeVisible(ceilingLabel);
  addAndMakeVisible(attackLabel);
  addAndMakeVisible(releaseLabel);
  addAndMakeVisible(ratioLabel);
  addAndMakeVisible(kneeLabel);
  addAndMakeVisible(grALabel);
  addAndMakeVisible(grBLabel);
  addAndMakeVisible(inputALabel);
  addAndMakeVisible(inputBLabel);
  addAndMakeVisible(outputALabel);
  addAndMakeVisible(outputBLabel);
  
  // Create parameter attachments
  clipperTypeAttachment = std::make_unique<juce::ComboBoxParameterAttachment>(
    *dynamic_cast<juce::AudioParameterChoice*>(vts.getParameter("clipperType")), clipperTypeBox);
  limiterTypeAttachment = std::make_unique<juce::ComboBoxParameterAttachment>(
    *dynamic_cast<juce::AudioParameterChoice*>(vts.getParameter("limiterType")), limiterTypeBox);
  thresholdAttachment = std::make_unique<juce::SliderParameterAttachment>(*vts.getParameter("threshold"), thresholdSlider);
  ceilingAttachment = std::make_unique<juce::SliderParameterAttachment>(*vts.getParameter("ceiling"), ceilingSlider);
  attackAttachment = std::make_unique<juce::SliderParameterAttachment>(*vts.getParameter("attack"), attackSlider);
  releaseAttachment = std::make_unique<juce::SliderParameterAttachment>(*vts.getParameter("release"), releaseSlider);
  ratioAttachment = std::make_unique<juce::SliderParameterAttachment>(*vts.getParameter("ratio"), ratioSlider);
  kneeAttachment = std::make_unique<juce::SliderParameterAttachment>(*vts.getParameter("knee"), kneeSlider);
}

void CrossFXAudioProcessorEditor::paint(juce::Graphics& g)
{
  g.fillAll(juce::Colour(0xff1a1a1a));
  g.setColour(juce::Colours::white);
  g.setFont(16.0f);
  // Remove title text; logo serves as brand/title
  if (logoImage.isValid())
  {
    auto panel = juce::Rectangle<float>(10, 48, (float)getWidth()-20, 130.0f);
    lnF.drawBevelPanel(g, panel, 10.0f, false);
    // Center the logo within the full panel area (no right-side reservation)
    auto logoBounds = juce::Rectangle<int>( (int)panel.getX() + 8, (int)panel.getY() + 8,
                                            (int)panel.getWidth() - 16, (int)panel.getHeight() - 16 );
    g.drawImageWithin(logoImage, logoBounds.getX(), logoBounds.getY(), logoBounds.getWidth(), logoBounds.getHeight(), juce::RectanglePlacement::centred);
  }
  g.setFont(12.0f);
  // Place A/B labels tight to the slider ends
  auto sliderBounds = blendSlider.getBounds();
  g.drawText("A", sliderBounds.getX() - 18, sliderBounds.getY() - 8, 18, 16, juce::Justification::centredRight);
  g.drawText("B", sliderBounds.getRight() + 2, sliderBounds.getY() - 8, 18, 16, juce::Justification::centredLeft);

  auto meterArea = getLocalBounds().reduced(12).removeFromBottom(140);
  const int meterWidth = 56;
  const int meterGap = 40;
  int cx = meterArea.getCentreX();
  auto meterA = juce::Rectangle<int>(cx - meterGap/2 - meterWidth, meterArea.getY() + 10, meterWidth, meterArea.getHeight() - 20);
  auto meterB = juce::Rectangle<int>(cx + meterGap/2,           meterArea.getY() + 10, meterWidth, meterArea.getHeight() - 20);
  // Beveled meter panels
  lnF.drawBevelPanel(g, meterA.toFloat().expanded(6), 6.0f, true);
  lnF.drawBevelPanel(g, meterB.toFloat().expanded(6), 6.0f, true);
  // Rollback: no alignment panel
  drawMeterTicks(g, meterA);
  drawMeterTicks(g, meterB);
  drawMeter(g, meterA, audioProcessor.getInputAPeak(), audioProcessor.getInputAClipped() ? juce::Colours::red : juce::Colours::lightgreen);
  drawMeter(g, meterB, audioProcessor.getInputBPeak(), audioProcessor.getInputBClipped() ? juce::Colours::red : juce::Colours::orange);
  g.drawText("A", meterA.withHeight(20).translated(0, -18), juce::Justification::centred);
  g.drawText("B", meterB.withHeight(20).translated(0, -18), juce::Justification::centred);
}

void CrossFXAudioProcessorEditor::resized()
{
  auto area = getLocalBounds().reduced(12);
  auto top = area.removeFromTop(110);
  auto blendArea = top.removeFromLeft(area.getWidth() - 220).reduced(10, 28);
  blendSlider.setBounds(blendArea);
  // Place Fade dropdown on the right side, below the Auto Gain button, not overlapping anything
  {
    const int rightMargin = 12;
    const int fadeWidth = 180;
    const int fadeHeight = 24;
    const int fadeX = getWidth() - rightMargin - fadeWidth;
    const int fadeY = 56; // below top-right button (which sits at y=12, h=28)
    auto fadeArea = juce::Rectangle<int>(fadeX, fadeY, fadeWidth, fadeHeight);
    fadeModeBox.setBounds(fadeArea);
    // Label under dropdown
    auto* fadeLabel = dynamic_cast<juce::Label*>(findChildWithID("fadeLabel"));
    if (fadeLabel == nullptr)
    {
      auto lbl = std::make_unique<juce::Label>();
      lbl->setComponentID("fadeLabel");
      lbl->setText("Fade Type", juce::dontSendNotification);
      lbl->setJustificationType(juce::Justification::centred);
      lbl->setColour(juce::Label::textColourId, juce::Colours::white);
      addAndMakeVisible(lbl.get());
      lbl.release();
    }
    if (auto* lbl = dynamic_cast<juce::Label*>(findChildWithID("fadeLabel")))
      lbl->setBounds(fadeArea.translated(0, fadeHeight + 2).withHeight(18));
  }

  // Clipper/Limiter section
  auto clipperSection = area.removeFromBottom(200);
  
  // Top row: Combo boxes
  auto comboRow = clipperSection.removeFromTop(60);
  auto clipperComboArea = comboRow.removeFromLeft(150).reduced(5);
  auto limiterComboArea = comboRow.removeFromLeft(150).reduced(5);
  
  clipperTypeBox.setBounds(clipperComboArea);
  limiterTypeBox.setBounds(limiterComboArea);
  
  // Bottom row: Sliders
  auto sliderRow = clipperSection.removeFromBottom(120);
  const int sliderWidth = 80;
  const int sliderSpacing = 10;
  int sliderX = 10;
  
  // Threshold and Ceiling
  auto thresholdArea = juce::Rectangle<int>(sliderX, 0, sliderWidth, 100);
  thresholdSlider.setBounds(thresholdArea);
  thresholdLabel.setBounds(thresholdArea.translated(0, 100).withHeight(20));
  sliderX += sliderWidth + sliderSpacing;
  
  auto ceilingArea = juce::Rectangle<int>(sliderX, 0, sliderWidth, 100);
  ceilingSlider.setBounds(ceilingArea);
  ceilingLabel.setBounds(ceilingArea.translated(0, 100).withHeight(20));
  sliderX += sliderWidth + sliderSpacing;
  
  // Attack and Release
  auto attackArea = juce::Rectangle<int>(sliderX, 0, sliderWidth, 100);
  attackSlider.setBounds(attackArea);
  attackLabel.setBounds(attackArea.translated(0, 100).withHeight(20));
  sliderX += sliderWidth + sliderSpacing;
  
  auto releaseArea = juce::Rectangle<int>(sliderX, 0, sliderWidth, 100);
  releaseSlider.setBounds(releaseArea);
  releaseLabel.setBounds(releaseArea.translated(0, 100).withHeight(20));
  sliderX += sliderWidth + sliderSpacing;
  
  // Ratio and Knee
  auto ratioArea = juce::Rectangle<int>(sliderX, 0, sliderWidth, 100);
  ratioSlider.setBounds(ratioArea);
  ratioLabel.setBounds(ratioArea.translated(0, 100).withHeight(20));
  sliderX += sliderWidth + sliderSpacing;
  
  auto kneeArea = juce::Rectangle<int>(sliderX, 0, sliderWidth, 100);
  kneeSlider.setBounds(kneeArea);
  kneeLabel.setBounds(kneeArea.translated(0, 100).withHeight(20));
  
  // Original gain knobs
  auto knobRow = area.removeFromBottom(160);
  auto leftKnob = knobRow.removeFromLeft(120).reduced(10);
  auto rightKnob = knobRow.removeFromRight(120).reduced(10);
  // Reserve space above knobs for dB labels
  auto leftLabel = leftKnob.removeFromTop(24);
  auto rightLabel = rightKnob.removeFromTop(24);
  dbALabel.setBounds(leftLabel);
  dbBLabel.setBounds(rightLabel);
  gainASlider.setBounds(leftKnob);
  gainBSlider.setBounds(rightKnob);

  // Position Auto Gain button at top-right, away from the centered logo panel and fade dropdown
  auto btnBounds = juce::Rectangle<int>(getWidth() - 120, 12, 100, 28);
  autoGainButton.setBounds(btnBounds);
}

void CrossFXAudioProcessorEditor::timerCallback()
{
  repaint();
  // Update dB labels (post-gain peaks)
  dbALabel.setText(juce::String(audioProcessor.getInputAdB(), 1) + " dB", juce::dontSendNotification);
  dbBLabel.setText(juce::String(audioProcessor.getInputBdB(), 1) + " dB", juce::dontSendNotification);
  
  // Update clipper/limiter meter labels
  grALabel.setText(juce::String(audioProcessor.clipperAGainReduction.load(), 1) + " dB", juce::dontSendNotification);
  grBLabel.setText(juce::String(audioProcessor.clipperBGainReduction.load(), 1) + " dB", juce::dontSendNotification);
  inputALabel.setText(juce::String(audioProcessor.clipperAInputLevel.load(), 1) + " dB", juce::dontSendNotification);
  inputBLabel.setText(juce::String(audioProcessor.clipperBInputLevel.load(), 1) + " dB", juce::dontSendNotification);
  outputALabel.setText(juce::String(audioProcessor.clipperAOutputLevel.load(), 1) + " dB", juce::dontSendNotification);
  outputBLabel.setText(juce::String(audioProcessor.clipperBOutputLevel.load(), 1) + " dB", juce::dontSendNotification);
}

void CrossFXAudioProcessorEditor::drawMeter(juce::Graphics& g, juce::Rectangle<int> bounds, float peak, juce::Colour fill)
{
  g.setColour(juce::Colours::black);
  g.fillRect(bounds);
  g.setColour(fill);
  const int h = juce::jlimit(0, bounds.getHeight(), (int) std::round(peak * bounds.getHeight()));
  g.fillRect(bounds.withY(bounds.getBottom() - h).withHeight(h));
  g.setColour(juce::Colours::grey);
  g.drawRect(bounds);
}

void CrossFXAudioProcessorEditor::drawMeterTicks(juce::Graphics& g, juce::Rectangle<int> bounds)
{
  // Draw -24, -12, -6, -3, 0 dB ticks (approximate linear mapping for simplicity)
  g.setColour(juce::Colours::grey.withAlpha(0.5f));
  auto drawTick = [&](float dB, int len)
  {
    // Approximate mapping: linear amplitude ~ 10^(dB/20)
    float amp = std::pow(10.0f, dB / 20.0f);
    int y = bounds.getBottom() - (int) std::round(amp * bounds.getHeight());
    g.drawLine((float)bounds.getX(), (float)y, (float)bounds.getX() + (float)len, (float)y);
  };
  drawTick(-24.0f, 8);
  drawTick(-12.0f, 10);
  drawTick(-6.0f, 12);
  drawTick(-3.0f, 12);
  drawTick(0.0f, 14);
}

// no XY pad in A/B mode


