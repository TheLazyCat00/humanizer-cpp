#pragma once

#include "KnobWithEditor.h"
#include "PluginProcessor.h"
#include "Types.h"

struct Knobs {
	KnobWithEditor range;
	KnobWithEditor center;
	KnobWithEditor speed;

	Knobs(APVTS& apvts)
			: range(apvts, PluginConfig::range.name, 120, 120)
			, center(apvts, PluginConfig::center.name, 120, 120)
			, speed(apvts, PluginConfig::speed.name, 120, 120)
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
