#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"

Humanizer::Humanizer()
		: AudioProcessor (BusesProperties()
			.withInput("Input", AudioChannelSet::stereo(), true)
			.withOutput ("Output", AudioChannelSet::stereo(), true))
		, apvts(
			* this,
			nullptr,
			"PARAMETERS",
			createParameterLayout()) {
	parameters.forEach([this](Parameter& p) {
		p.link(apvts, getSampleRate());
	});
}

Humanizer::~Humanizer() {
}

AudioProcessorValueTreeState::ParameterLayout Humanizer::createParameterLayout() {
	std::vector<std::unique_ptr<RangedAudioParameter>> params;

	parameters.forEach([&params] (Parameter& parameter) {
		params.push_back(std::make_unique<AudioParameterFloat>(
			ParameterID { parameter.settings.name, 1 },
			parameter.settings.name,
			NormalisableRange<float>(parameter.settings.min, parameter.settings.max),
			parameter.settings.defaultVal
		));
	});

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
	parameters.forEach([&sampleRate] (Parameter& parameter) {
		parameter.smoothed.reset(sampleRate, 0.05);
	});
}

void Humanizer::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages) {
	parameters.forEach([] (Parameter& p) {
		if (p.parameter) // Always check for null!
			p.smoothed.setTargetValue(p.parameter->load());
	});

	// 2. Audio Loop
	for (int i = 0; i < buffer.getNumSamples(); ++i) {
		float currentRange = parameters.range.smoothed.getNextValue();
		
		for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
			buffer.getWritePointer(channel)[i] *= (currentRange / 50.0f);
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
	parameters.forEach([this] (Parameter& parameter) {
		parameter.smoothed.reset(getSampleRate(), 0.05);
	});
}
