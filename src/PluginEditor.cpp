#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Editor::Editor(Humanizer& p)
		: AudioProcessorEditor (&p)
		, processorRef(p)
		, knobs(p.apvts) {
	setSize(400, 300);
	setResizable(true, true);

	knobs.forEach([this] (KnobWithEditor& knob) {
		addAndMakeVisible(knob);
	});

	setLookAndFeel(&modernLook);
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

	FlexBox knobsContainer;
	knobsContainer.flexDirection = FlexBox::Direction::column;
	knobsContainer.flexWrap = FlexBox::Wrap::noWrap;
	knobsContainer.justifyContent = FlexBox::JustifyContent::center;
	knobsContainer.alignContent = FlexBox::AlignContent::center;

	// Add items by passing the component into the FlexItem constructor
	// then chain the layout settings (margin, flexGrow, etc.)
	knobsContainer.items.add(FlexItem(knobs.range).withMargin(5).withFlex(1.0f));
	knobsContainer.items.add(FlexItem(knobs.center).withMargin(5).withFlex(1.0f));
	knobsContainer.items.add(FlexItem(knobs.speed).withMargin(5).withFlex(1.0f));

	knobsContainer.performLayout(area);
}
