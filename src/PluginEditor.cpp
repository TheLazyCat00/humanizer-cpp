#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Editor::Editor(Humanizer& p)
		: AudioProcessorEditor (&p)
		, processorRef(p)
		, knobs(p.apvts) 
		, diagram(-50, 50) {
	setSize(600, 400);
	setResizable(true, true);

	addAndMakeVisible(diagram);
	knobs.forEach([this] (KnobWithEditor& knob) {
		addAndMakeVisible(knob);
	});

	setLookAndFeel(&modernLook);

	diagram.updateData({ 10.0f, -20.0f, 45.0f, 5.0f, -30.0f });
}

Editor::~Editor() {
	setLookAndFeel (nullptr);
}

//==============================================================================
void Editor::paint(Graphics& g) {
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void Editor::resized() {
	auto area = getLocalBounds().reduced(20);

	// 1. Setup the Knobs Container
	FlexBox knobsContainer;
	knobsContainer.flexDirection = FlexBox::Direction::column;
	// ... your existing knob setup ...
	knobsContainer.items.add(FlexItem(knobs.range).withMargin(5).withMinHeight(50).withFlex(1.0f));
	knobsContainer.items.add(FlexItem(knobs.center).withMargin(5).withMinHeight(50).withFlex(1.0f));
	knobsContainer.items.add(FlexItem(knobs.speed).withMargin(5).withMinHeight(50).withFlex(1.0f));

	// 2. Setup the Main Viewport
	FlexBox viewport;
	viewport.flexDirection = FlexBox::Direction::row;

	// Give the knobs a fixed width, and let the diagram take the rest of the space
	viewport.items.add(FlexItem(knobsContainer).withWidth(150.0f).withMargin(5));
	viewport.items.add(FlexItem(diagram).withFlex(1.0f).withMargin(5));

	viewport.performLayout(area);
}
