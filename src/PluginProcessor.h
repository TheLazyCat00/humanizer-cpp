// PluginProcessor.h
#pragma once
#include <JuceHeader.h>
#include "Types.h"
#include "PluginConfig.h"

inline float hashToFloat(int seed, int index, int subSeed) {
	unsigned int x = static_cast<unsigned int>(seed + index + subSeed);

	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = (x >> 16) ^ x;

	return (static_cast<float>(x) / static_cast<float>(std::numeric_limits<unsigned int>::max()) * 2.0f) - 1.0f;
}

class Humanizer;

class BezierGenerator {
	struct Anchor {
		float value;
		float tension;
	};

	Humanizer& humanizer;
public:
	int seed;

	BezierGenerator(Humanizer& h, int seed) : humanizer(h) {
		this->seed = seed;
	}

	// Helper to generate a deterministic random float based on seed and segment index
	float getDeterministicValue(int segmentIndex, int subSeed) const{
		return hashToFloat(seed, segmentIndex, subSeed);
	}

	float getNormalized(double currentBeat);
	double getValue(double currentBeat);
};

class Humanizer : public AudioProcessor {
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Humanizer);
	dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::Linear> delayLine;

public:
	Humanizer();
	~Humanizer() override;

	AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;
	double getRequiredLatencyMs() const;
	bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
	void processBlock(AudioBuffer<float>&, MidiBuffer&) override;
	using AudioProcessor::processBlock;

	AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override;
	const String getName() const override { return JucePlugin_Name; };
	bool acceptsMidi() const override { return false; };
	bool producesMidi() const override { return false; };
	bool isMidiEffect() const override { return false; };
	double getTailLengthSeconds() const override { return 0.0; };

	int getNumPrograms() override { return 1; };
	int getCurrentProgram() override { return 1; };
	void setCurrentProgram(int index) override { ignoreUnused(index); };
	const String getProgramName(int index) override { ignoreUnused (index); return {}; };
	void changeProgramName(int index, const String& newName) override { ignoreUnused(index, newName); };

	void getStateInformation(MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

	Parameters parameters;
	APVTS apvts;
	BezierGenerator bezierGen;
};

//==============================================================================

inline float BezierGenerator::getNormalized(double currentBeat) {
	float speedBeats = humanizer.parameters.speed.smoothed.getCurrentValue();
	speedBeats = std::max(0.1f, speedBeats);

	// 1. Calculate which segment we are in and the 0.0-1.0 phase within it
	double segmentFloat = currentBeat / speedBeats;
	int segmentIndex = static_cast<int>(std::floor(segmentFloat));
	float t = static_cast<float>(segmentFloat - segmentIndex); // This is our 'phase'

	// 2. Generate deterministic Anchors for THIS segment and the NEXT
	// Using sub-seeds (e.g. 100, 200) to get different random properties for same segment
	Anchor p0 = { getDeterministicValue(segmentIndex, 0),     // Value
		std::abs(getDeterministicValue(segmentIndex, 100)) * 0.45f }; // Tension

	Anchor p1 = { getDeterministicValue(segmentIndex + 1, 0), 
		std::abs(getDeterministicValue(segmentIndex + 1, 100)) * 0.45f };

	// 3. Cubic Bezier Interpolation
	float invT = 1.0f - t;

	// Simplified Cubic Bezier (where control point Y = anchor Y)
	float val = (invT * invT * invT * p0.value) +
		(3.0f * invT * invT * t * p0.value) +
		(3.0f * t * t * invT * p1.value) +
		(t * t * t * p1.value);

	return val;
}

inline double BezierGenerator::getValue(double currentBeat) {
	float range = humanizer.parameters.range.smoothed.getCurrentValue();
	float center = humanizer.parameters.center.smoothed.getCurrentValue();

	double noise = getNormalized(currentBeat);
	return range * 0.5 * (center + noise);
}
