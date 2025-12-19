#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Editor::Editor(Humanizer& p)
		: AudioProcessorEditor (&p)
		, range(p.apvts, "range")
		, processorRef(p) {
	setSize(400, 300);
	setResizable(true, true);

	addAndMakeVisible(range);
}

Editor::~Editor() {
}

//==============================================================================
void Editor::paint (Graphics& g) {
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	

	// g.fillAll(getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void Editor::resized() {
	// This is generally where you'll want to lay out the positions of any
	// subcomponents in your editor..
	auto area = getLocalBounds().reduced(20);
    range.setBounds(area.removeFromTop(200));
}
