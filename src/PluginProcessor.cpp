#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"

Humanizer::Humanizer()
		: AudioProcessor (BusesProperties()
			.withInput("Input", AudioChannelSet::stereo(), true)
			.withOutput ("Output", AudioChannelSet::stereo(), true)),
		apvts(
			* this,
			nullptr,
			"PARAMETERS",
			createParameterLayout()
		) {
    parameters.range = apvts.getRawParameterValue("range");
	jassert(parameters.range != nullptr);

	smoothed.range.reset(getSampleRate(), 0.05); // 50 ms smoothing
	smoothed.range.setCurrentAndTargetValue(parameters.range->load());
}

Humanizer::~Humanizer() {
}

AudioProcessorValueTreeState::ParameterLayout Humanizer::createParameterLayout() {
	std::vector<std::unique_ptr<RangedAudioParameter>> params;

	params.push_back(std::make_unique<AudioParameterFloat>(
		ParameterID { "range", 1 },           // ID (must be stable!)
		"Range",                              // Name
		NormalisableRange<float>(0.0f, 50.0f),
		0.0f                                  // Default
	));

	return { params.begin(), params.end() };
}

bool Humanizer::isBusesLayoutSupported (const BusesLayout& layouts) const {
	// This is the place where you check if the layout is supported.
	// In this template code we only support mono or stereo.
	// Some plugin hosts, such as certain GarageBand versions, will only
	// load plugins that support stereo bus layouts.
	if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
		&& layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
		return false;

	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;

	return true;
}

void Humanizer::prepareToPlay(double sampleRate, int) {
	smoothed.range.reset(sampleRate, 0.05);
}

void Humanizer::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages) {
	ignoreUnused(midiMessages);

	smoothed.range.setTargetValue(parameters.range->load());

	for (int channel = 0; channel < buffer.getNumChannels(); ++ channel) {
		auto* data = buffer.getWritePointer(channel);

		for (int i = 0; i < buffer.getNumSamples(); ++i)
		{
			float currentRange = smoothed.range.getNextValue();
			data[i] *= currentRange / 50.0f; // example gain scaling
		}
	}
}

bool Humanizer::hasEditor() const {
	return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor * Humanizer::createEditor() {
	return new Editor(* this);
}

//==============================================================================
void Humanizer::getStateInformation(MemoryBlock& destData) {
	auto state = apvts.copyState();
	std::unique_ptr<XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void Humanizer::setStateInformation(const void * data, int sizeInBytes) {
	std::unique_ptr<XmlElement> xml(
		getXmlFromBinary(data, sizeInBytes));

	if (xml != nullptr)
		apvts.replaceState(ValueTree::fromXml(*xml));
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor * JUCE_CALLTYPE createPluginFilter() {
	return new Humanizer();
}


void Humanizer::releaseResources() {
	smoothed.range.reset(getSampleRate(), 0.05);
}
