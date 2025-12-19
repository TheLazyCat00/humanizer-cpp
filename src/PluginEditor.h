#pragma once

#include "KnobWithEditor.h"
#include "PluginProcessor.h"
#include "Types.h"

struct Knobs {
	KnobWithEditor range;
	KnobWithEditor center;
	KnobWithEditor speed;

	Knobs(APVTS& apvts)
		: range(apvts, PluginConfig::range.name, 150, 150)
		, center(apvts, PluginConfig::center.name, 150, 150)
		, speed(apvts, PluginConfig::speed.name, 150, 150)
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
	ModernLookAndFeel modernLook;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Editor)
public:
	explicit Editor(Humanizer&);
	~Editor() override;

	//==============================================================================
	void paint (Graphics&) override;
	void resized() override;
	Knobs knobs;
};
