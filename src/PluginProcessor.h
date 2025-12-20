#pragma once
#include <JuceHeader.h>
#include "Types.h"
#include "PluginConfig.h"

class Humanizer;

class Perlin {
	double currentPhase = 0.0f;
	Humanizer& humanizer;

public:
	float uniqueSeed = 0.0f;
	Perlin(Humanizer& h) : humanizer(h) {
		juce::Random r;
		uniqueSeed = r.nextFloat() * 1000.0f;
	}

	float getSmoothNoise(float x);
	float getNormalized(double bpm, double sampleRate);
	double getValue(double bpm, double sampleRate);
};

class Humanizer : public AudioProcessor {
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Humanizer);
	dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::Linear> delayLine;
public:
	Humanizer();
	~Humanizer() override;

	AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
	//==============================================================================
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;
	double getRequiredLatencyMs() const;

	bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

	void processBlock(AudioBuffer<float>&, MidiBuffer&) override;
	using AudioProcessor::processBlock;

	//==============================================================================
	AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override;
	//==============================================================================
	const String getName() const override { return JucePlugin_Name; };

	bool acceptsMidi() const override { return false; };
	bool producesMidi() const override { return false; };
	bool isMidiEffect() const override { return false; };
	double getTailLengthSeconds() const override { return 0.0; };

	//==============================================================================
	int getNumPrograms() override { return 1; };
	int getCurrentProgram() override { return 1; };
	void setCurrentProgram(int index) override { ignoreUnused(index); };
	const String getProgramName(int index) override { ignoreUnused (index); return {}; };
	void changeProgramName(int index, const String& newName) override { ignoreUnused(index, newName); };

	//==============================================================================
	void getStateInformation(MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

	Parameters parameters;
	APVTS apvts;
	SmoothedValue<float> smoothedDelay;
	Perlin perlin;
	std::atomic<float> currentNoiseForDisplay { 0.0f };
};

inline float Perlin::getSmoothNoise(float x) {
	float shiftedX = x + uniqueSeed;
	int x0 = static_cast<int>(shiftedX);
	int x1 = x0 + 1;
	float frac = shiftedX - static_cast<float>(x0);

	auto getHash = [](int v) {
		v = (v << 13) ^ v;
		return (1.0f - ((v * (v * v * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
	};

	float t = frac * frac * (3.0f - 2.0f * frac); 
	return getHash(x0) * (1.0f - t) + getHash(x1) * t;
}

inline float Perlin::getNormalized(double bpm, double sampleRate) {
    float speedBeats = humanizer.parameters.speed.smoothed.getNextValue();
    double beatsPerSecond = bpm / 60.0;
    double frequencyHz = beatsPerSecond / std::max(0.1f, speedBeats);

    // 1. Advance the Master Phase
    currentPhase += frequencyHz / sampleRate;
    if (currentPhase > 1.0) currentPhase -= 1.0;

    // 2. Slow "Amplitude Evolution"
    // We want the 'mix' of these sines to change slowly over time
    // so the pattern never feels like a loop.
    amplitudePhase += 0.1 / sampleRate; // Very slow drift
    if (amplitudePhase > 1.0) amplitudePhase -= 1.0;

    const float twoPi = juce::MathConstants<float>::twoPi;

    // 3. Create the sines with shifting weights
    // We use slow LFOs (or just more sines) to modulate the amplitudes
    float amp1 = std::sin(twoPi * amplitudePhase * 0.13f) * 0.5f + 0.5f; // Rises/falls over seconds
    float amp2 = std::sin(twoPi * amplitudePhase * 0.27f) * 0.3f + 0.3f;
    float amp3 = std::sin(twoPi * amplitudePhase * 0.41f) * 0.2f + 0.2f;

    // Ratio 1 (The Main Sway)
    float val = std::sin(twoPi * currentPhase) * amp1;
    
    // Ratio 2 (The Double-time Jitter)
    val += std::sin(twoPi * currentPhase * 2.0f) * amp2;
    
    // Ratio 0.5 (The Long-form Drift)
    val += std::sin(twoPi * currentPhase * 0.5f) * amp3;

    // 4. Final Normalization
    // Sum of max possible amplitudes is approx 1.0 + 0.6 + 0.4 = 2.0
    return val * 0.5f; 
}

inline double Perlin::getValue(double bpm, double sampleRate) {
	float range = humanizer.parameters.range.parameter->load();
	float center = humanizer.parameters.center.parameter->load();
	
	double noise = getNormalized(bpm, sampleRate);
	return range * 0.5 * (center + noise);
};
