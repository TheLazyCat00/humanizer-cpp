#include "PluginProcessor.h"
#include "Types.h"
#include "PluginEditor.h"

void ModernLookAndFeel::drawRotarySlider(
		Graphics& g, int x, int y, int width, int height,
		float sliderPosProportional, float rotaryStartAngle,
		float rotaryEndAngle, Slider& slider) {
	auto radius = jmin(width / 2, height / 2) - 4.0f;
	auto centreX = x + width / 2;
	auto centreY = y + height / 2;
	auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

	g.setColour(Colours::violet);

	int numSegments = 100;

	Path backgroundArc;
	for (int i = 0; i <= numSegments; ++ i) {
		float px = centreX + radius * std::cos(rotaryEndAngle - MathConstants<float>::halfPi);
		float py = centreY + radius * std::sin(rotaryEndAngle - MathConstants<float>::halfPi);
		if (i == 0)
			backgroundArc.startNewSubPath(px, py);
		else
			backgroundArc.lineTo(px, py);
	}

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

	g.strokePath(valueArc, PathStrokeType(3.0f, PathStrokeType::curved, PathStrokeType::rounded));
};

//==============================================================================
Editor::Editor(Humanizer& p)
		: AudioProcessorEditor (&p),
		processorRef(p) {
	setSize(400, 300);

	range.setSliderStyle(Slider::RotaryVerticalDrag);
	modernLook = std::make_unique<ModernLookAndFeel>();
	range.setLookAndFeel(modernLook.get());

	addAndMakeVisible(range);

	attachements.range =
	std::make_unique<APVTS::SliderAttachment>(
		processorRef.apvts,
		"range",
		range);

}

Editor::~Editor() {
}

//==============================================================================
void Editor::paint (Graphics& g) {
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	

	g.fillAll(getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void Editor::resized() {
	// This is generally where you'll want to lay out the positions of any
	// subcomponents in your editor..
	auto area = getLocalBounds().reduced(20);
    range.setBounds(area.removeFromTop(200));
}
