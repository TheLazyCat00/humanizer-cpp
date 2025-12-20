#include "PluginEditor.h"

//==============================================================================
Editor::Editor(Humanizer& p)
		: AudioProcessorEditor (&p)
		, processorRef(p)
		, knobs(p.apvts)
		, diagram(- PluginConfig::range.min, PluginConfig::range.max, 120) {
	setSize(600, 400);
	setResizable(true, true);
	setLookAndFeel(&modernLook);

	addAndMakeVisible(diagram);
	knobs.forEach([this] (KnobWithEditor& knob) {
		addAndMakeVisible(knob);
	});

	updateDiagramLimits();

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
	float val = processorRef.currentNoiseForDisplay.load();

	diagram.shift(val);
	diagram.updateSmoothing(); 
}

void Editor::parameterChanged(const String& parameterID, float newValue) {
	MessageManager::callAsync([this]() {
		updateDiagramLimits();
	});
}

void Editor::updateDiagramLimits() {
	float r = processorRef.parameters.range.parameter->load();
	float c = processorRef.parameters.center.parameter->load();

	float theoreticalMax = c + (r * 0.5f);
	float theoreticalMin = c - (r * 0.5f);

	float padding = r * 0.1f;

	diagram.setLimits(theoreticalMin - padding, theoreticalMax + padding);
}
