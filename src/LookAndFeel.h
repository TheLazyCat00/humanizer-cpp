// LookAndFeel.h
#pragma once
#include <JuceHeader.h>

namespace ModernTheme {
	static const Colour& mainAccent = Colours::purple;
	static const Colour& background = Colour(0xFF121212);
}

class ModernLookAndFeel : public LookAndFeel_V4 {
public:
	ModernLookAndFeel() {
		setColour(ResizableWindow::backgroundColourId, ModernTheme::background);
		setColour(Label::textColourId, Colours::white);
	}

	void drawRotarySlider(
			Graphics& g, int x, int y, int width, int height,
			float sliderPosProportional, float rotaryStartAngle,
			float rotaryEndAngle, Slider& slider) override {
		auto radius = jmin(width / 2, height / 2) - 4.0f;
		auto centreX = x + width / 2.0f;
		auto centreY = y + height / 2.0f;
		auto angle = rotaryStartAngle + sliderPosProportional *(rotaryEndAngle - rotaryStartAngle);
		float innerStrokeThickness = radius * 0.15f;
		float outerStrokeThickness = radius * 0.15f;

		Path backgroundArc;
		backgroundArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, 
			rotaryStartAngle, rotaryEndAngle, true);

		float innerRad = radius - (innerStrokeThickness * 0.5f);
		float outerRad = radius + (innerStrokeThickness * 0.5f);

		Colour outerColor = Colours::white.darker(0.9f);
		Colour innerColor = Colours::darkgrey.darker(1);
		ColourGradient outlineGradient(innerColor, centreX, centreY,
			outerColor, centreX + outerRad, centreY, true);

		float stopPosition = innerRad / outerRad;
		outlineGradient.addColour(stopPosition, innerColor);

		g.setGradientFill(outlineGradient);
		g.strokePath(backgroundArc, PathStrokeType(outerStrokeThickness, PathStrokeType::curved, PathStrokeType::rounded));

		float innerStrokeRadius = radius -(outerStrokeThickness - innerStrokeThickness) / 2;
		Path valueArc;
		valueArc.addCentredArc(centreX, centreY, innerStrokeRadius, innerStrokeRadius, 0.0f,
			rotaryStartAngle, angle, true);

		g.setColour(ModernTheme::mainAccent);
		g.strokePath(valueArc, PathStrokeType(innerStrokeThickness, PathStrokeType::curved, PathStrokeType::rounded));

		auto lightDiameter = (radius * 2.0f) * 0.7f;
		auto lightRadius = lightDiameter / 2;
		
		ColourGradient ellipseGradient(Colours::darkgrey.darker(1.0f - sliderPosProportional * 0.7f), centreX, centreY,
			Colours::black, centreX + innerRad, centreY, true);
		g.setGradientFill(ellipseGradient);
		g.fillEllipse(centreX - lightRadius, centreY - lightRadius, lightDiameter, lightDiameter);
	}

	Label * createSliderTextBox(Slider& slider) override {
		auto* label = LookAndFeel_V4::createSliderTextBox(slider);

		label->setJustificationType(Justification::centred);
		label->setFont(FontOptions(14.0f, Font::bold));
		label->setColour(Label::textColourId, Colours::white);
		label->setColour(Label::backgroundColourId, Colours::transparentBlack);
		label->setColour(Label::outlineColourId, Colours::transparentBlack);

		return label;
	}
};
