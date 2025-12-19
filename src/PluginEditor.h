#pragma once

#include "KnobWithEditor.h"
#include "PluginProcessor.h"
#include "LookAndFeel.h"

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
	KnobWithEditor range;
	KnobWithEditor center;
	KnobWithEditor speed;
};
