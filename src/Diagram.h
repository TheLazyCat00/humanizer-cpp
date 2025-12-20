#pragma once

#include <JuceHeader.h>
#include "LookAndFeel.h"

class DiagramComponent : public Component {
	float min, max;
	std::vector<float> dataPoints;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DiagramComponent)
public:
	DiagramComponent(float minValue, float maxValue)
		: min(minValue), max(maxValue)
	{
	}

	void updateData(const std::vector<float>& newData)
	{
		dataPoints = newData;
		repaint(); 
	}

	void paint(Graphics& g) override
	{
		auto bounds = getLocalBounds().toFloat();
		float margin = 10.0f; // Space to keep text away from the very edge
		
		// 1. Draw Background
		g.fillAll(Colours::black);

		// 2. Draw Min/Max Labels
		g.setColour(Colours::white.withAlpha(0.7f));
		g.setFont(14.0f);

		// Max value at the top left
		g.drawText(String(max, 1), 
					margin, margin, 100, 20, 
					Justification::topLeft);

		// Min value at the bottom left
		g.drawText(String(min, 1), 
					margin, bounds.getHeight() - margin - 20.0f, 100, 20, 
					Justification::bottomLeft);

		if(dataPoints.size() < 2) return;

		g.setColour(ModernTheme::mainAccent);
		Path diagramPath;
		
		auto getX = [this, bounds](int index) {
			return jmap((float)index, 0.0f,(float)(dataPoints.size() - 1), 0.0f, bounds.getWidth());
		};

		auto getY = [this, bounds](float value) {
			return jmap(value, min, max, bounds.getHeight(), 0.0f);
		};

		diagramPath.startNewSubPath(getX(0), getY(dataPoints[0]));
		for(int i = 1; i < dataPoints.size(); ++i)
		{
			diagramPath.lineTo(getX(i), getY(dataPoints[i]));
		}

		g.strokePath(diagramPath, PathStrokeType(2.0f));
	}
};
