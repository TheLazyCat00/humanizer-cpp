#pragma once
#include <JuceHeader.h>
#include "Types.h"

class Parameter {
public:
	std::atomic<float> * parameter;
	SmoothedValue<float> smoothed;

	Parameter(String name, APVTS& apvts, double sampleRate) {
		parameter = apvts.getRawParameterValue(name);
		jassert(parameter != nullptr);

		smoothed.reset(sampleRate, 0.05);
		smoothed.setCurrentAndTargetValue(parameter->load());
	}
};

struct Parameters {
	Parameter range;
	Parameter center;
	Parameter speed;

	Parameters(APVTS& apvts, double sampleRate)
		: range("range", apvts, sampleRate)
		, center("center", apvts, sampleRate)
		, speed("speed", apvts, sampleRate) {
	}

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

	AudioProcessorValueTreeState apvts;
	Parameters parameters;
};
