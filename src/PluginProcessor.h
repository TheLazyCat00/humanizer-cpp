#pragma once
#include <JuceHeader.h>

struct Parameters {
	std::atomic<float> * range;
};

struct Smoothed {
	SmoothedValue<float> range;
};

class Humanizer : public AudioProcessor {
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
	Smoothed smoothed;
private:
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Humanizer)
};
