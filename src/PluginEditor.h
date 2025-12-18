#pragma once

#include "Types.h"
#include "PluginProcessor.h"

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

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Editor)
};
