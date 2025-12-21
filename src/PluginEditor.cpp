// PluginEditor.cpp
#include "PluginEditor.h"
#include "PluginConfig.h"

//==============================================================================
Editor::Editor(Humanizer& p)
		: AudioProcessorEditor (&p)
		, processorRef(p)
		, knobs(p.apvts)
		, diagram(p) {
	openGLContext.attachTo(* this);
	openGLContext.setContinuousRepainting(true);
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
	openGLContext.detach();
}

//==============================================================================
void Editor::paint(Graphics& g) {
	g.fillAll(ModernTheme::background);
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

	if (limitsDirty.exchange(false)) {
		updateDiagramLimits();
	}
	diagram.updateSmoothing();
}

void Editor::parameterChanged(const String& parameterID, float newValue) {
	limitsDirty = true;
}

void Editor::updateDiagramLimits() {
	float r = processorRef.parameters.range.parameter->load();
	float c = processorRef.parameters.center.parameter->load();


	float theoreticalMax = 0.5 * r * (c + 1);
	float theoreticalMin = 0.5 * r * (c - 1);

	diagram.setLimits(theoreticalMin, theoreticalMax);
}
