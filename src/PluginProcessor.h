// PluginProcessor.h
#pragma once
#include <JuceHeader.h>
#include <limits>
#include <algorithm>
#include <cmath>
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

	double segmentFloat = currentBeat / speedBeats;
	int segmentIndex = static_cast<int>(std::floor(segmentFloat));
	float t = static_cast<float>(segmentFloat - segmentIndex);

	// 1. Get Values and Tensions
	float y0 = getDeterministicValue(segmentIndex, 0);
	float y3 = getDeterministicValue(segmentIndex + 1, 0);

	// 1. Get raw random values [0.0, 1.0]
	float rawHashOut = std::abs(getDeterministicValue(segmentIndex, 100));
	float rawHashIn  = std::abs(getDeterministicValue(segmentIndex + 1, 100));

	// 2. Define your range
	constexpr float minTension = 0.1f;
	constexpr float maxTension = 0.45f;

	// 3. Map the [0.0, 1.0] range to [0.1, 0.45]
	float tensionOut = jmap(rawHashOut, 0.0f, 1.0f, minTension, maxTension);
	float tensionIn  = jmap(rawHashIn,  0.0f, 1.0f, minTension, maxTension);

	// 2. Adjust the weights based on tension
	float invT = 1.0f - t;

	float w1 = 3.0f * (1.0f + tensionOut * 5.0f);
	float w2 = 3.0f * (1.0f + tensionIn * 5.0f);

	// 3. Calculate weighted Bezier
	float term0 = invT * invT * invT;
	float term1 = w1 * invT * invT * t;
	float term2 = w2 * t * t * invT;
	float term3 = t * t * t;

	float totalWeight = term0 + term1 + term2 + term3;

	return (term0 * y0 + term1 * y0 + term2 * y3 + term3 * y3) / totalWeight;
}

inline double BezierGenerator::getValue(double currentBeat) {
	float range = humanizer.parameters.range.smoothed.getCurrentValue();
	float center = humanizer.parameters.center.smoothed.getCurrentValue();

	double noise = getNormalized(currentBeat);
	return range * 0.5 * (center + noise);
}
