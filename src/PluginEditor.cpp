// PluginEditor.cpp
#include <vector>
#include "PluginEditor.h"
#include "PluginConfig.h"
#include "Defer.h"

//==============================================================================
Editor::Editor(Humanizer& p)
		: AudioProcessorEditor (&p)
		, processorRef(p)
		, knobs(p.apvts)
		, diagram() {
	openGLContext.attachTo(* this);
	setSize(600, 400);
	setResizable(true, true);
	setResizeLimits(300, 250, 1200, 800);

	setLookAndFeel(&modernLook);
	tooltipWindow->setMillisecondsBeforeTipAppears(1500);

	processorRef.apvts.addParameterListener(PluginConfig::range.name, this);
	processorRef.apvts.addParameterListener(PluginConfig::center.name, this);

	updateDiagramLimits();

	addAndMakeVisible(diagram);
	knobs.forEach([this] (KnobWithEditor& knob) {
		addAndMakeVisible(knob);
	});

	startTimerHz(30);
}

Editor::~Editor() {
	openGLContext.detach();
	setLookAndFeel(nullptr);
	processorRef.apvts.removeParameterListener(PluginConfig::range.name, this);
    processorRef.apvts.removeParameterListener(PluginConfig::center.name, this);
}

void Editor::paint(Graphics& g) {
	g.fillAll(ModernTheme::background);
}

void Editor::resized() {
	auto area = getLocalBounds().reduced(20);

	float availableHeight = (float)area.getHeight();
	float idealKnobWidth = (availableHeight / 3.0f);
	
	float dynamicWidth = jmin(130.0f, idealKnobWidth);

	FlexBox knobsContainer;
	knobsContainer.flexDirection = FlexBox::Direction::column;
	knobsContainer.justifyContent = FlexBox::JustifyContent::spaceBetween;
	
	const FlexItem::Margin knobMargin = FlexItem::Margin(0, 0, 15, 0);
	knobsContainer.items.add(FlexItem(knobs.range)
		.withFlex(1.0f)
		.withMinWidth(50.0f)
		.withMargin(knobMargin));
	knobsContainer.items.add(FlexItem(knobs.center)
		.withFlex(1.0f)
		.withMinWidth(50.0f)
		.withMargin(knobMargin));
	knobsContainer.items.add(FlexItem(knobs.speed)
		.withFlex(1.0f)
		.withMinWidth(50.0f));

	FlexBox viewport;
	viewport.flexDirection = FlexBox::Direction::row;

	viewport.items.add(FlexItem(knobsContainer)
		.withFlex(0.0f, 1.0f, dynamicWidth));

	viewport.items.add(FlexItem().withWidth(20));

	viewport.items.add(FlexItem(diagram)
		.withFlex(3.0f));

	viewport.performLayout(area);
}

void Editor::timerCallback() {
	if (!isShowing()) return;

	if (limitsDirty.exchange(false)) {
		updateDiagramLimits();
	}

	defer {
		diagram.updateSmoothing();
		diagram.repaint();
	};

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

	const double pixelsPerQuarter = double(diagram.getWidth()) / 8;

	// Use a double for accumulation to avoid losing sub-pixel movement
	static double pixelAccumulator = 0.0;
	pixelAccumulator += quartersTraveled * pixelsPerQuarter;

	int pixelsToShift = static_cast<int>(pixelAccumulator);

	std::vector<float> valuesToDraw;
	if (pixelsToShift > 0) {
		pixelAccumulator -= pixelsToShift;
		double beatStep = quartersTraveled / pixelsToShift;

		for (int i = 1; i <= pixelsToShift; ++i) {
			double sampleTime = lastPlayHeadPos + (beatStep * i);
			valuesToDraw.push_back((float)processorRef.bezierGen.getValue(sampleTime));
		}
		diagram.shift(valuesToDraw); // Call once!
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
