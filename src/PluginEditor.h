#pragma once

#include "KnobWithEditor.h"
#include "PluginProcessor.h"
#include "LookAndFeel.h"
#include "Types.h"

struct Knobs {
	KnobWithEditor range;
	KnobWithEditor center;
	KnobWithEditor speed;

	Knobs(APVTS& apvts)
		: range(apvts, "range", 150, 150)
		, center(apvts, "center", 150, 150)
		, speed(apvts, "speed", 150, 150)
		{
	}

	void forEach(std::function<void(KnobWithEditor&)> callback) {
		callback(range);
		callback(center);
		callback(speed);
	}
};

//==============================================================================
class Editor final : public AudioProcessorEditor {
	// This reference is provided as a quick way for your editor to
	// access the processor object that created it.
	Humanizer& processorRef;
	std::unique_ptr<ModernLookAndFeel> modernLook;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Editor)
public:
	explicit Editor(Humanizer&);
	~Editor() override;

	//==============================================================================
	void paint (Graphics&) override;
	void resized() override;
	Knobs knobs;
};
