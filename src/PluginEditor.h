#pragma once

#include "Types.h"
#include "PluginProcessor.h"

class ModernLookAndFeel : public LookAndFeel_V4 {
public:
	void drawRotarySlider(
		Graphics& g, int x, int y, int width, int height,
		float sliderPosProportional, float rotaryStartAngle,
		float rotaryEndAngle, Slider& slider
	) override;
};

struct Attachements {
	std::unique_ptr<APVTS::SliderAttachment> range;
};
//==============================================================================
class Editor final : public AudioProcessorEditor {
public:
	explicit Editor(Humanizer&);
	~Editor() override;

	//==============================================================================
	void paint (Graphics&) override;
	void resized() override;
	Slider range;
	Attachements attachements;
private:
	// This reference is provided as a quick way for your editor to
	// access the processor object that created it.
	Humanizer& processorRef;
	std::unique_ptr<ModernLookAndFeel> modernLook;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Editor)
};
