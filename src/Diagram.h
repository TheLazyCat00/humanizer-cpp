#pragma once

#include <JuceHeader.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include "LookAndFeel.h"
#include "Defer.h"

class Diagram : public Component {
	// Data storage: Use a circular buffer to avoid expensive vector erasures
	std::vector<float> dataBuffer;
	int writeIndex = 0;
	int totalPoints = 0;

	SmoothedValue<float, ValueSmoothingTypes::Linear> smoothMin, smoothMax;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Diagram)

public:
	Diagram() {
		setOpaque(true);
		smoothMin.reset(30, 0.2);
		smoothMax.reset(30, 0.2);
		smoothMin.setCurrentAndTargetValue(0);
		smoothMax.setCurrentAndTargetValue(1);
	}

	void shift(const std::vector<float>& values) {
		if (dataBuffer.empty()) return;

		// Add new values to circular buffer
		for (float v : values) {
			dataBuffer[writeIndex] = v;
			writeIndex = (writeIndex + 1) % dataBuffer.size();
			totalPoints = std::min((int)dataBuffer.size(), totalPoints + 1);
		}
	}

	void paint(Graphics& g) override {
		auto bounds = getLocalBounds().toFloat();
		g.fillAll(Colours::black);
		defer {
			drawOverlays(g, bounds);
		};

		if (totalPoints < 2) return;

		// 1. Prepare the Path
		Path graphPath;
		graphPath.preallocateSpace(totalPoints * 3); // Optimization: avoid reallocs

		float min = smoothMin.getCurrentValue();
		float max = smoothMax.getCurrentValue();
		float w = bounds.getWidth();
		float h = bounds.getHeight();

		// 2. Iterate through circular buffer to build path
		// We start from the oldest point and go to the newest
		for (int i = 0; i < totalPoints; ++i) {
			// Calculate index in circular buffer
			int bufferIdx = (writeIndex - totalPoints + i + (int)dataBuffer.size()) % (int)dataBuffer.size();

			float x = w - (totalPoints - i);
			float yVal = jmap(dataBuffer[bufferIdx], min, max, h - 2.0f, 2.0f);
			float yPos = jlimit(0.0f, h, yVal);

			if (i == 0)
				graphPath.startNewSubPath(x, yPos);
			else
				graphPath.lineTo(x, yPos);
		}

		// 3. Draw the Path (Stroking is usually faster than filling a complex shape)
		g.setColour(ModernTheme::mainAccent);
		g.strokePath(graphPath, PathStrokeType(1.5f, PathStrokeType::curved, PathStrokeType::rounded));
	}

	void drawOverlays(Graphics& g, Rectangle<float> bounds) {
		float margin = 10.0f;
		float zeroY = jmap(0.0f, smoothMin.getCurrentValue(), smoothMax.getCurrentValue(), bounds.getHeight(), 0.0f);

		g.setColour(Colours::white.withAlpha(0.4f));
		float dashPattern[] = { 4.0f, 4.0f };
		g.drawDashedLine(Line<float>(0, zeroY, bounds.getWidth(), zeroY), dashPattern, 2, 1.0f);

		g.setColour(Colours::white.withAlpha(0.7f));
		g.setFont(14.0f);
		g.drawText(String(smoothMax.getTargetValue(), 1) + " ms", margin, 2, 100, 20, Justification::topLeft);
		g.drawText(String(smoothMin.getTargetValue(), 1) + " ms", margin, bounds.getHeight() - 22, 100, 20, Justification::bottomLeft);
	}

	void updateSmoothing() {
		if (smoothMin.isSmoothing() || smoothMax.isSmoothing()) {
			smoothMin.getNextValue();
			smoothMax.getNextValue();
		}
	}

	void setLimits(float min, float max) {
		if (std::abs(max - min) < 0.001f) max = min + 0.1f;
		smoothMin.setTargetValue(min);
		smoothMax.setTargetValue(max);
	}

	void resized() override {
		int w = getWidth();
		if (w > 0) {
			dataBuffer.assign(w, 0.0f); // Size buffer to match width
			writeIndex = 0;
			totalPoints = 0;
		}
	}
};
