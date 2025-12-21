// PluginEditor.cpp
#include "PluginEditor.h"
#include "PluginConfig.h"

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

	auto playHead = processorRef.getPlayHead();
	if (playHead == nullptr) return;
	auto position = playHead->getPosition();
	if (!position.hasValue()) return;

	double currentBeat = position->getPpqPosition().orFallback(0.0);

	// 1. Calculate delta
	double quartersTraveled = currentBeat - lastPlayHeadPos;

	// Handle wrap-around/loops/stops
	if (quartersTraveled <= 0 || quartersTraveled > 1.0) {
		lastPlayHeadPos = currentBeat;
		return;
	}

	// 2. Define "Zoom": How many pixels wide is one quarter note?
	const double pixelsPerQuarter = double(diagram.getWidth()) / 4;

	// Use a double for accumulation to avoid losing sub-pixel movement
	static double pixelAccumulator = 0.0;
	pixelAccumulator += quartersTraveled * pixelsPerQuarter;

	int pixelsToShift = static_cast<int>(pixelAccumulator);

	if (pixelsToShift > 0) {
		// Subtract the whole pixels we are about to process from the accumulator
		pixelAccumulator -= pixelsToShift;

		// 3. Step through the gap
		double beatStep = quartersTraveled / pixelsToShift;

		for (int i = 1; i <= pixelsToShift; ++i) {
			// Sample from the last position up to the current position
			double sampleTime = lastPlayHeadPos + (beatStep * i);
			float val = (float)processorRef.bezierGen.getValue(sampleTime);
			diagram.shift(val);
		}
	}

	lastPlayHeadPos = currentBeat;
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
