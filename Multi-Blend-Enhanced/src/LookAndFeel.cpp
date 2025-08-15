#include "LookAndFeel.h"

MultiBlenderLookAndFeel::MultiBlenderLookAndFeel()
{
  setColour(juce::ResizableWindow::backgroundColourId, background());
  setColour(juce::Slider::trackColourId, primary().withAlpha(0.8f));
  setColour(juce::Slider::thumbColourId, primary());
  setColour(juce::Slider::textBoxTextColourId, text());
  setColour(juce::Slider::textBoxBackgroundColourId, surface());
}

void MultiBlenderLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                        float sliderPos, float, float,
                        const juce::Slider::SliderStyle style, juce::Slider& s)
{
  auto bounds = juce::Rectangle<float>(x, y, width, height).reduced(2);
  auto isHorizontal = style == juce::Slider::LinearHorizontal;

  // Beveled track panel
  auto track = isHorizontal ? bounds.withHeight(10.0f).withY(bounds.getCentreY() - 5.0f)
                            : bounds.withWidth(10.0f).withX(bounds.getCentreX() - 5.0f);
  drawBevelPanel(g, track, 5.0f, true);

  // Fill
  g.setColour(primary().brighter(0.2f));
  if (isHorizontal)
  {
    auto fill = track.withRight(sliderPos);
    g.fillRoundedRectangle(fill, 3.0f);
  }
  else
  {
    auto fill = track.withY(sliderPos).withHeight(track.getBottom() - sliderPos);
    g.fillRoundedRectangle(fill, 3.0f);
  }

  // Thumb with shadow
  juce::DropShadow ds(juce::Colours::black.withAlpha(0.6f), 8, {0, 2});
  juce::Path thumb;
  float tx = isHorizontal ? sliderPos : track.getCentreX();
  float ty = isHorizontal ? track.getCentreY() : sliderPos;
  thumb.addEllipse(tx - 8.0f, ty - 8.0f, 16.0f, 16.0f);
  ds.drawForPath(g, thumb);
  juce::ColourGradient knobGrad(juce::Colours::white, tx - 4.0f, ty - 4.0f,
                                juce::Colours::lightgrey, tx + 6.0f, ty + 6.0f, true);
  g.setGradientFill(knobGrad);
  g.fillPath(thumb);
  g.setColour(juce::Colours::black.withAlpha(0.6f));
  g.strokePath(thumb, juce::PathStrokeType(1.0f));
}

void MultiBlenderLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool,
                    int, int, int, int, juce::ComboBox& box)
{
  auto r = juce::Rectangle<float>(0, 0, (float)width, (float)height).reduced(1.0f);
  g.setColour(surface());
  g.fillRoundedRectangle(r, 6.0f);
  g.setColour(juce::Colours::black.withAlpha(0.5f));
  g.drawRoundedRectangle(r, 6.0f, 1.0f);
}

void MultiBlenderLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& b,
                            const juce::Colour& backgroundColour,
                            bool shouldDrawButtonAsHighlighted,
                            bool shouldDrawButtonAsDown)
{
  auto r = b.getLocalBounds().toFloat();
  juce::DropShadow ds(juce::Colours::black.withAlpha(0.6f), 8, {0, 2});
  juce::Path p; p.addRoundedRectangle(r, 6.0f);
  ds.drawForPath(g, p);
  auto col = backgroundColour;
  if (shouldDrawButtonAsDown) col = col.darker(0.2f);
  if (shouldDrawButtonAsHighlighted) col = col.brighter(0.1f);
  g.setColour(col);
  g.fillPath(p);
}

juce::Font MultiBlenderLookAndFeel::getLabelFont(juce::Label&)
{
  return juce::Font(13.0f);
}

juce::Font MultiBlenderLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
  return juce::Font(13.0f);
}

void MultiBlenderLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                        float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                        juce::Slider&)
{
  auto bounds = juce::Rectangle<float>(x, y, width, height).reduced(2);
  auto r = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
  auto centre = bounds.getCentre();

  // Body with subtle radial gradient for 3D
  juce::ColourGradient grad(knobFace().brighter(0.15f), centre.x - r*0.5f, centre.y - r*0.5f,
                            knobFace().darker(0.35f), centre.x + r*0.7f, centre.y + r*0.7f, true);
  g.setGradientFill(grad);
  g.fillEllipse(centre.x - r, centre.y - r, 2*r, 2*r);
  // Inner highlight ring
  g.setColour(knobRing());
  g.drawEllipse(centre.x - r + 2.0f, centre.y - r + 2.0f, 2*(r-2.0f), 2*(r-2.0f), 2.0f);

  // Indicator
  auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
  juce::Point<float> p(centre.x + std::cos(angle) * (r - 6.0f), centre.y + std::sin(angle) * (r - 6.0f));
  g.setColour(knobMarker());
  g.drawLine(centre.x, centre.y, p.x, p.y, 3.0f);
}

void MultiBlenderLookAndFeel::drawBevelPanel(juce::Graphics& g, juce::Rectangle<float> bounds, float radius, bool recessed) const
{
  // Outer shadow/highlight
  juce::DropShadow ds(juce::Colours::black.withAlpha(0.6f), recessed ? 10 : 6, {0, recessed ? 3 : 2});
  juce::Path path; path.addRoundedRectangle(bounds, radius);
  ds.drawForPath(g, path);

  // Base fill gradient
  auto light = surface().brighter(0.15f);
  auto dark  = surface().darker(0.35f);
  juce::ColourGradient grad(recessed ? dark : light, bounds.getX(), bounds.getY(),
                            recessed ? light : dark, bounds.getRight(), bounds.getBottom(), false);
  g.setGradientFill(grad);
  g.fillPath(path);

  // Inner stroke for bevel
  g.setColour(juce::Colours::black.withAlpha(0.4f));
  g.strokePath(path, juce::PathStrokeType(1.0f));
}


