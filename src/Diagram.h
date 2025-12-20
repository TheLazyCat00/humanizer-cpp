#pragma once

#include <JuceHeader.h>
#include "LookAndFeel.h"

class DiagramComponent : public Component {
	// We split these into 'target' (what the knobs say) 
	// and 'current' (the smoothed value for the UI)
	float targetMin, targetMax;
	float currentMin, currentMax;
	
	// 0.1f is a fast ease-out. Lower is smoother/slower.
	const float lerpFactor = 0.1f; 

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DiagramComponent)
public:
	Array<float> dataPoints;

	DiagramComponent(float minValue, float maxValue, int length)
		: targetMin(minValue), targetMax(maxValue), 
		  currentMin(minValue), currentMax(maxValue) {
		dataPoints.insertMultiple(0, 0, length);
	}

	void shift(float value) {
		dataPoints.add(value);
		dataPoints.remove(0);
		// We don't call repaint here because the Timer in the Editor 
		// will trigger it via updateSmoothing().
	}

	// Call this from the Editor's timerCallback!
	void updateSmoothing() {
		currentMin += (targetMin - currentMin) * lerpFactor;
		currentMax += (targetMax - currentMax) * lerpFactor;

		// Only repaint if the values are still moving significantly or the data is shifting
		repaint();
	}

	void paint(Graphics& g) override {
		auto bounds = getLocalBounds().toFloat();
		float margin = 10.0f;
		
		g.fillAll(Colours::black);

		// 1. Draw "Zero Line" (Original Timing)
		// This helps the user see early vs late shifts
		g.setColour(Colours::white.withAlpha(0.1f));
		float zeroY = jmap(0.0f, currentMin, currentMax, bounds.getHeight(), 0.0f);
		// 1. Define the source line
		Path zeroLinePath;
		zeroLinePath.startNewSubPath(0, zeroY);
		zeroLinePath.lineTo(bounds.getWidth(), zeroY);

		// 2. Define a path to hold the generated dashes
		Path dashedPath;

		// 3. Define the dash pattern (4 pixels on, 4 pixels off)
		float dashPattern[] = { 4.0f, 4.0f };

		// 4. Generate the dashed stroke into 'dashedPath'
		PathStrokeType(1.0f).createDashedStroke(dashedPath, zeroLinePath, dashPattern, 2);

		// 5. Finally, fill the generated path (not stroke it, because it's already a "stroke shape")
		g.fillPath(dashedPath);

		// 2. Draw Smoothed Min/Max Labels
		g.setColour(Colours::white.withAlpha(0.7f));
		g.setFont(14.0f);
		g.drawText(String(targetMax, 1) + " ms", margin, margin, 100, 20, Justification::topLeft);
		g.drawText(String(targetMin, 1) + " ms", margin, bounds.getHeight() - margin - 20.0f, 100, 20, Justification::bottomLeft);

		if(dataPoints.size() < 2) return;

		// 3. Draw the Humanizer Path
		g.setColour(ModernTheme::mainAccent);
		Path diagramPath;
		
		auto getX = [this, bounds](int index) {
			return jmap((float)index, 0.0f, (float)(dataPoints.size() - 1), 0.0f, bounds.getWidth());
		};

		auto getY = [this, bounds](float value) {
			// We use currentMin/currentMax for the smooth zoom effect
			return jmap(value, currentMin, currentMax, bounds.getHeight(), 0.0f);
		};

		diagramPath.startNewSubPath(getX(0), getY(dataPoints[0]));
		for(int i = 1; i < dataPoints.size(); ++i) {
			diagramPath.lineTo(getX(i), getY(dataPoints[i]));
		}

		g.strokePath(diagramPath, PathStrokeType(2.0f, PathStrokeType::curved));
	}

	void setLimits(float min, float max) {
		if (std::abs(max - min) < 0.001f) {
			max = min + 0.1f;
		}

		targetMin = min;
		targetMax = max;
	}
};
