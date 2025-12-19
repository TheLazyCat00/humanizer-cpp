#pragma once
#include <JuceHeader.h>

class ModernLookAndFeel : public LookAndFeel_V4 {
public:
	void drawRotarySlider(
			Graphics& g, int x, int y, int width, int height,
			float sliderPosProportional, float rotaryStartAngle,
			float rotaryEndAngle, Slider& slider) override {
		auto radius = jmin(width / 2, height / 2) - 4.0f;
		auto centreX = x + width / 2;
		auto centreY = y + height / 2;
		auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

		int numSegments = 100;

		Path backgroundArc;
		for (int i = 0; i <= numSegments; ++ i) {
			float t = i / (float) numSegments;
			float a = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
			float px = centreX + radius * std::cos(a - MathConstants<float>::halfPi);
			float py = centreY + radius * std::sin(a - MathConstants<float>::halfPi);
			if (i == 0)
				backgroundArc.startNewSubPath(px, py);
			else
				backgroundArc.lineTo(px, py);
		}

		g.setColour(Colours::grey);
		g.strokePath(backgroundArc, PathStrokeType(3.0f, PathStrokeType::curved, PathStrokeType::rounded));

		Path valueArc;
		for (int i = 0; i <= numSegments; ++ i) {
			float t = i / (float) numSegments;
			float a = rotaryStartAngle + t * (angle - rotaryStartAngle);
			float px = centreX + radius * std::cos(a - MathConstants<float>::halfPi);
			float py = centreY + radius * std::sin(a - MathConstants<float>::halfPi);
			if (i == 0)
				valueArc.startNewSubPath(px, py);
			else
				valueArc.lineTo(px, py);
		}

		g.setColour(Colours::violet);
		g.strokePath(valueArc, PathStrokeType(3.0f, PathStrokeType::curved, PathStrokeType::rounded));
	}

	Label * createSliderTextBox (Slider& slider) override {
		auto* label = LookAndFeel_V4::createSliderTextBox(slider);

		label->setJustificationType(Justification::centred);
		label->setFont(FontOptions(14.0f, Font::bold));
		label->setColour(Label::textColourId, Colours::white);
		label->setColour(Label::backgroundColourId, Colours::transparentBlack);
		label->setColour(Label::outlineColourId, Colours::transparentBlack);

		return label;
	}
};
