#include "PluginProcessor.h"
#include "Types.h"
#include "PluginEditor.h"

//==============================================================================
Editor::Editor(Humanizer& p)
		: AudioProcessorEditor (&p),
		processorRef(p) {
	setSize(400, 300);

	range.setSliderStyle(Slider::RotaryVerticalDrag);
	addAndMakeVisible(range);

	attachements.range = std::make_unique<
		APVTS::SliderAttachment>(
			processorRef.apvts,
			"range",
			range);
}

Editor::~Editor() {
}

//==============================================================================
void Editor::paint (Graphics& g) {
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	

	g.fillAll(getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

	g.setColour (Colours::white);
	g.setFont(15.0f);
	g.drawFittedText("Hello World!", getLocalBounds(), Justification::centred, 1);
}

void Editor::resized() {
	// This is generally where you'll want to lay out the positions of any
	// subcomponents in your editor..
	auto area = getLocalBounds().reduced(20);
    range.setBounds(area.removeFromTop(200));
}
