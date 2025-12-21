// Diagram.h
#pragma once

#include <JuceHeader.h>
#include "LookAndFeel.h"

class DiagramComponent : public Component {
	SmoothedValue<float, ValueSmoothingTypes::Linear> smoothMin, smoothMax;
	int writePos = 0;
	Image canvas;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DiagramComponent)
public:
	DiagramComponent(float minValue, float maxValue)
	: canvas(Image::ARGB, 1, 1, true) { // Size updated in resized()
		setOpaque(true);
		smoothMin.reset(60, 0.2);
		smoothMax.reset(60, 0.2);
		smoothMin.setCurrentAndTargetValue(minValue);
		smoothMax.setCurrentAndTargetValue(maxValue);

		// Optimize image for pixel access
		canvas.clear(canvas.getBounds(), Colours::black);
	}

	void shift(float value) {
		if (canvas.isNull()) return;

		// 1. Update Smoothing
		if (smoothMin.isSmoothing() || smoothMax.isSmoothing()) {
			smoothMin.getNextValue();
			smoothMax.getNextValue();
		}

		// 2. Map value to Y coordinate
		float min = smoothMin.getCurrentValue();
		float max = smoothMax.getCurrentValue();
		int h = canvas.getHeight();

		// Map value to pixel height (inverted because Y=0 is top)
		int y = jmap(value, min, max, (float)h, 0.0f);
		y = jlimit(0, h - 1, y);

		// 3. Write directly to the Image pixels (Fastest possible method)
		// We write a vertical "scanline" at the current writePos
// In shift():
{
    // Write directly to ARGB memory
    juce::Image::BitmapData data(canvas, juce::Image::BitmapData::writeOnly);
    
    // 1. Fast Column Clear using raw pointers
    // We iterate down the column to clear the previous trace
    for (int y = 0; y < h; ++y) {
        // Get pointer to the pixel at (writePos, y)
        auto* pixel = (uint32*)data.getPixelPointer(writePos, y);
        *pixel = 0xFF121212; // Write Background Color directly (ARGB hex)
    }

    // 2. Draw the new dot
    auto* pixel = (uint32*)data.getPixelPointer(writePos, y);
    *pixel = 0xFF9D00FF; // Write Accent Color (Purple) directly
}

		// 4. Increment and wrap the write head
		writePos = (writePos + 1) % canvas.getWidth();

		// Note: We do NOT call repaint() here. 
		// The Timer in PluginEditor calls repaint(), which is correct.
	}

	void paint(Graphics& g) override {
		if (canvas.isNull()) return;

		// Since we used setOpaque(true), we don't need to fill the background
		// unless the image doesn't cover the whole area (which it does).

		int w = getWidth();
		int h = getHeight();
		int rightChunkWidth = w - writePos;

		// Draw the scrolled image
		g.drawImage(canvas, 
			  0, 0, rightChunkWidth, h, 
			  writePos, 0, rightChunkWidth, h);

		g.drawImage(canvas, 
			  rightChunkWidth, 0, writePos, h, 
			  0, 0, writePos, h);

		// Optional: Draw a "Playhead" line to mask the seam
		// g.setColour(Colours::white.withAlpha(0.2f));
		// g.drawVerticalLine(rightChunkWidth, 0, (float)h);
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
			writePos = 0;
		}
	}
};
