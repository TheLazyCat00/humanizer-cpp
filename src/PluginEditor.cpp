// PluginEditor.cpp
#include "PluginEditor.h"

//==============================================================================
Editor::Editor(Humanizer& p)
		: AudioProcessorEditor (&p)
		, processorRef(p)
		, knobs(p.apvts)
		, diagram(- PluginConfig::range.min, PluginConfig::range.max) {
	openGLContext.attachTo(* this);
	setSize(600, 400);
	setResizable(true, true);
	setLookAndFeel(&modernLook);

	processorRef.apvts.addParameterListener(PluginConfig::range.name, this);
    processorRef.apvts.addParameterListener(PluginConfig::center.name, this);
	addAndMakeVisible(diagram);
	knobs.forEach([this] (KnobWithEditor& knob) {
		addAndMakeVisible(knob);
	});

	startTimerHz(60);
}

Editor::~Editor() {
	setLookAndFeel(nullptr);
	processorRef.apvts.removeParameterListener(PluginConfig::range.name, this);
    processorRef.apvts.removeParameterListener(PluginConfig::center.name, this);
}

//==============================================================================
void Editor::paint(Graphics& g) {
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void Editor::resized() {
	auto area = getLocalBounds().reduced(20);

	FlexBox knobsContainer;
	knobsContainer.flexDirection = FlexBox::Direction::column;
	knobsContainer.items.add(FlexItem(knobs.range).withMargin(5).withMinHeight(50).withFlex(1.0f));
	knobsContainer.items.add(FlexItem(knobs.center).withMargin(5).withMinHeight(50).withFlex(1.0f));
	knobsContainer.items.add(FlexItem(knobs.speed).withMargin(5).withMinHeight(50).withFlex(1.0f));

	FlexBox viewport;
	viewport.flexDirection = FlexBox::Direction::row;

	viewport.items.add(FlexItem(knobsContainer).withWidth(150.0f).withMargin(5));
	viewport.items.add(FlexItem(diagram).withFlex(1.0f).withMargin(5));

	viewport.performLayout(area);
}

void Editor::timerCallback() {
	if (!isShowing()) return;

	// Handle updates 60 times a second, instead of 1000 times a second
	if (limitsDirty.exchange(false)) {
		updateDiagramLimits();
	}

	float val = processorRef.currentNoiseForDisplay.load();
	diagram.shift(val);
	diagram.repaint();
}

void Editor::parameterChanged(const String& parameterID, float newValue) {
	limitsDirty = true;
}

void Editor::updateDiagramLimits() {
	float r = processorRef.parameters.range.parameter->load();
	float c = processorRef.parameters.center.parameter->load();


	float theoreticalMax = 0.5 * r * (c + 1);
	float theoreticalMin = 0.5 * r * (c - 1);

	float padding = r * 0.1f;

	diagram.setLimits(theoreticalMin - padding, theoreticalMax + padding);
}
