#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class MultiBlenderLookAndFeel : public juce::LookAndFeel_V4
{
public:
  MultiBlenderLookAndFeel();
  ~MultiBlenderLookAndFeel() override = default;

  void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                        float sliderPos, float minSliderPos, float maxSliderPos,
                        const juce::Slider::SliderStyle, juce::Slider& slider) override;

  void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                    int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override;

  void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                            const juce::Colour& backgroundColour,
                            bool shouldDrawButtonAsHighlighted,
                            bool shouldDrawButtonAsDown) override;

  juce::Font getLabelFont(juce::Label&) override;
  juce::Font getComboBoxFont(juce::ComboBox&) override;

  // Palette
  juce::Colour background() const { return juce::Colour(0xff141414); }
  juce::Colour surface() const { return juce::Colour(0xff1f1f1f); }
  juce::Colour primary() const { return juce::Colour(0xff6ab04a); }
  juce::Colour accent() const { return juce::Colour(0xff00a8ff); }
  juce::Colour text() const { return juce::Colours::white; }

  // Vintage knob colors
  juce::Colour knobFace() const { return juce::Colour(0xff2b2b2b); }
  juce::Colour knobRing() const { return juce::Colour(0xffb1976b); }
  juce::Colour knobMarker() const { return juce::Colour(0xffe0c58a); }

  void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                        float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                        juce::Slider& slider) override;

  // Utility to draw a beveled panel (recessed/raised)
  void drawBevelPanel(juce::Graphics& g, juce::Rectangle<float> bounds, float radius,
                      bool recessed = true) const;
};


