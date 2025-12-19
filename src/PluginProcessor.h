#pragma once
#include <JuceHeader.h>
#include "Types.h"

struct Parameter {
	String name;
	float start, end, defaultValue;
	std::atomic<float>* parameter = nullptr; // Initialize to nullptr!
	SmoothedValue<float> smoothed;

	Parameter(String n, float s, float e, float d) 
		: name(n), start(s), end(e), defaultValue(d) {}

	void link(APVTS& apvts, double sampleRate) {
		parameter = apvts.getRawParameterValue(name);
		if (parameter) {
			smoothed.reset(sampleRate, 0.05);
			smoothed.setCurrentAndTargetValue(parameter->load());
		}
	}
};

struct Parameters {
	// Initialize with hardcoded values immediately
	Parameter range  { "range", 0.0f, 50.0f, 0.0f };
	Parameter center { "center", 0.0f, 1.0f, 0.5f };
	Parameter speed  { "speed", 1.0f, 16.0f, 1.0f };

	// Default constructor is fine now
	Parameters() {}

	void forEach(std::function<void(Parameter&)> callback) {
		callback(range);
		callback(center);
		callback(speed);
	}
};

class Humanizer : public AudioProcessor {
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Humanizer);
public:
	Humanizer();
	~Humanizer() override;

	AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
	//==============================================================================
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;

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
};
