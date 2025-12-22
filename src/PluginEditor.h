// PluginEditor.h
#pragma once

#include "KnobWithEditor.h"
#include "Diagram.h"
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
class Editor : public AudioProcessorEditor, public Timer, public AudioProcessorValueTreeState::Listener {
	Humanizer& processorRef;
	ModernLookAndFeel modernLook;
	OpenGLContext openGLContext;
	std::atomic<bool> limitsDirty;
	double lastPlayHeadPos = 0;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Editor)
public:
	explicit Editor(Humanizer&);
	~Editor() override;

	//==============================================================================
	void paint (Graphics&) override;
	void resized() override;
	void timerCallback() override;
	Knobs knobs;
	Diagram diagram;
	void parameterChanged (const juce::String& parameterID, float newValue) override;
    void updateDiagramLimits();
};
