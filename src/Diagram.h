// Diagram.h
#pragma once

#include <JuceHeader.h>
#include "LookAndFeel.h"

class Diagram : public Component {
	SmoothedValue<float, ValueSmoothingTypes::Linear> smoothMin, smoothMax;
	Image canvas;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Diagram)
public:
	Diagram()
	: canvas(Image::ARGB, 1, 1, true) { // Size updated in resized()
		setOpaque(true);
		smoothMin.reset(60, 0.2);
		smoothMax.reset(60, 0.2);
		smoothMin.setCurrentAndTargetValue(0);
		smoothMax.setCurrentAndTargetValue(1);

		// Optimize image for pixel access
		canvas.clear(canvas.getBounds(), Colours::black);
	}

	void shift(float value) {
    const int w = getWidth();
    const int h = getHeight();

    // Safety check: Don't process if image isn't ready
    if (w <= 0 || h <= 0 || canvas.isNull()) return;

    // 1. Move pixels
    canvas.moveImageSection(0, 0, 1, 0, w - 1, h);

    // 2. Clear new column and draw
    {
        Graphics g(canvas);
        
        // Clear the rightmost column
        g.setColour(Colours::black);
        g.drawVerticalLine(w - 1, 0.0f, (float)h);

        // Calculate Y position
        float min = smoothMin.getCurrentValue();
        float max = smoothMax.getCurrentValue();
        
        // Use float for jmap to keep precision, then round
        float yVal = jmap(value, min, max, (float)h - 1.0f, 0.0f);
        int yPos = jlimit(0, h - 1, roundToInt(yVal));
        
        // Use a 2x2 square instead of setPixelAt so it's actually visible!
        g.setColour(ModernTheme::mainAccent);
        g.fillRect(w - 2, yPos - 1, 2, 2); 
    }

    repaint();
}

	void paint(Graphics& g) override {
		g.fillAll(Colours::black);

		if (canvas.isValid()) {
			g.drawImageAt(canvas, 0, 0);
		}

		float margin = 10.0f;
		auto bounds = getLocalBounds().toFloat();
		g.setColour(Colours::white.withAlpha(0.6f));
		float zeroY = jmap(0.0f, smoothMin.getCurrentValue(), smoothMax.getCurrentValue(), bounds.getHeight(), 0.0f);

		Path zeroLinePath;
		zeroLinePath.startNewSubPath(0, zeroY);
		zeroLinePath.lineTo(bounds.getWidth(), zeroY);

		Path dashedPath;
		float dashPattern[] = { 4.0f, 4.0f };
		PathStrokeType(1.0f).createDashedStroke(dashedPath, zeroLinePath, dashPattern, 2);
		g.fillPath(dashedPath);

		g.setColour(Colours::white.withAlpha(0.7f));
		g.setFont(14.0f);
		g.drawText(String(smoothMin.getTargetValue(), 1) + " ms", margin, margin, 100, 20, Justification::topLeft);
		g.drawText(String(smoothMax.getTargetValue(), 1) + " ms", margin, bounds.getHeight() - margin - 20.0f, 100, 20, Justification::bottomLeft);
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
		if (getWidth() > 0 && getHeight() > 0) {
			// Ensure we recreate as ARGB
			Image newCanvas(Image::ARGB, getWidth(), getHeight(), true);
			newCanvas.clear(newCanvas.getBounds(), Colours::black);
			canvas = newCanvas;
		}
	}
};
