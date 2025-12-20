#pragma once

#include <JuceHeader.h>
#include "LookAndFeel.h"

class DiagramComponent : public juce::Component {
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothMin, smoothMax;
    
    juce::Array<float> dataPoints;
    int head = 0; // Points to the next index to be written

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DiagramComponent)

public:
    DiagramComponent(float minValue, float maxValue, int length) {
        // Initialize with zeros
        for (int i = 0; i < length; ++i) dataPoints.add(0.0f);
        
        // Setup smoothing
        smoothMin.reset(60, 0.2);
        smoothMax.reset(60, 0.2);
        smoothMin.setCurrentAndTargetValue(minValue);
        smoothMax.setCurrentAndTargetValue(maxValue);
    }

    void shift(float value) {
        dataPoints.set(head, value);
        head = (head + 1) % dataPoints.size();
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        float margin = 10.0f;
        
        // Use smoothed values for visual coordinate math
float currentMin = smoothMin.getCurrentValue();
    float currentMax = smoothMax.getCurrentValue();

        g.fillAll(juce::Colours::black);

        // 1. Draw "Zero Line" (Calculated based on smoothed zoom)
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        float zeroY = juce::jmap(0.0f, currentMin, currentMax, bounds.getHeight(), 0.0f);
        
        juce::Path zeroLinePath;
        zeroLinePath.startNewSubPath(0.0f, zeroY);
        zeroLinePath.lineTo(bounds.getWidth(), zeroY);

        juce::Path dashedPath;
        float dashPattern[] = { 4.0f, 4.0f };
        juce::PathStrokeType(1.0f).createDashedStroke(dashedPath, zeroLinePath, dashPattern, 2);
        g.fillPath(dashedPath);

        // 2. Draw Min/Max Labels
        g.setColour(juce::Colours::white.withAlpha(0.7f));
        g.setFont(14.0f);
        
        // We use TargetValue for labels so the text feels stable while the wave zooms
        juce::String maxStr = juce::String(smoothMax.getTargetValue(), 1) + " ms";
        juce::String minStr = juce::String(smoothMin.getTargetValue(), 1) + " ms";

        g.drawText(maxStr, margin, margin, 100, 20, juce::Justification::topLeft);
        g.drawText(minStr, margin, bounds.getHeight() - margin - 20.0f, 100, 20, juce::Justification::bottomLeft);

        if (dataPoints.size() < 2) return;

        // 3. Draw the Noise Path
        g.setColour(ModernTheme::mainAccent);
        juce::Path diagramPath;
        diagramPath.preallocateSpace(dataPoints.size() * 3);

        auto getX = [this, bounds](int iterationIndex) {
            return juce::jmap((float)iterationIndex, 0.0f, (float)(dataPoints.size() - 1), 0.0f, bounds.getWidth());
        };

        auto getY = [currentMin, currentMax, bounds](float value) {
            return juce::jmap(value, currentMin, currentMax, bounds.getHeight(), 0.0f);
        };

        // Start from oldest data (at 'head') and wrap around
        for (int i = 0; i < dataPoints.size(); ++i) {
            int bufferIndex = (head + i) % dataPoints.size();
            float val = dataPoints[bufferIndex];

            if (i == 0)
                diagramPath.startNewSubPath(getX(i), getY(val));
            else
                diagramPath.lineTo(getX(i), getY(val));
        }

        g.strokePath(diagramPath, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved));
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
};
