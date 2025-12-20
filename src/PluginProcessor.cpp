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
			createParameterLayout())
		, perlin(*this) {
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

double Humanizer::getRequiredLatencyMs() const {
	float r = parameters.range.parameter->load();
	float c = parameters.center.parameter->load();
	float worstCase = -r * 0.5 * (c - 1.0);
	worstCase = jmax(0.0f, worstCase);
	return worstCase;
}

void Humanizer::prepareToPlay(double sampleRate, int samplesPerBlock) {
	parameters.forEach([&sampleRate] (Parameter& parameter) {
		parameter.smoothed.reset(sampleRate, PluginConfig::ramptime);
	});

	smoothedDelay.reset (sampleRate, PluginConfig::ramptime); // 0.1s ramp time
	
	double latencyMs = getRequiredLatencyMs();
	int latencySamples = static_cast<int>((latencyMs / 1000.0) * sampleRate);
	
	setLatencySamples(latencySamples);

	dsp::ProcessSpec spec {
		sampleRate,
		static_cast<uint32>(samplesPerBlock),
		static_cast<uint32>(getTotalNumOutputChannels()),
	};

	delayLine.prepare(spec);
	
	float maxRangeMs = parameters.range.parameter->load();
	int maxSamplesNeeded = static_cast<int>(((latencyMs + maxRangeMs) / 1000.0) * sampleRate);
	delayLine.setMaximumDelayInSamples(maxSamplesNeeded + 100);
}

void Humanizer::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages) {
	double bpm = 120.0;
	if (auto* ph = getPlayHead())
		if (auto pos = ph->getPosition())
			bpm = pos->getBpm().orFallback(120.0);

	double sr = getSampleRate();
	double latencySamples = static_cast<double>(getLatencySamples());
	double latencyMs = (latencySamples / sr) * 1000.0;

	for (int i = 0; i < buffer.getNumSamples(); ++i) {
		double noiseShiftMs = perlin.getValue(bpm, sr);
			
		double finalDelayMs = latencyMs + noiseShiftMs;
		
		float delayInSamples = static_cast<float>((finalDelayMs / 1000.0) * sr);
		delayLine.setDelay(delayInSamples);

		for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
			delayLine.pushSample(ch, buffer.getSample(ch, i));
			buffer.setSample(ch, i, delayLine.popSample(ch));
		}

		if (i == buffer.getNumSamples() - 1)
			currentNoiseForDisplay.store(static_cast<float>(finalDelayMs));
	}
}

bool Humanizer::hasEditor() const {
	return true;
}

AudioProcessorEditor * Humanizer::createEditor() {
	return new Editor(* this);
}

//==============================================================================
void Humanizer::getStateInformation(MemoryBlock& destData) {
	auto state = apvts.copyState();
	state.setProperty("seed", perlin.uniqueSeed, nullptr);
	std::unique_ptr<XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void Humanizer::setStateInformation(const void * data, int sizeInBytes) {
	std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

	if (xml != nullptr) {
		auto tree = juce::ValueTree::fromXml(*xml);
		
		apvts.replaceState(tree);
		
		if (tree.hasProperty("seed")) {
			perlin.uniqueSeed = static_cast<float>(tree.getProperty("seed"));
		}
	}
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor * JUCE_CALLTYPE createPluginFilter() {
	return new Humanizer();
}

void Humanizer::releaseResources() {
	parameters.forEach([this] (Parameter& parameter) {
		parameter.smoothed.reset(getSampleRate(), PluginConfig::ramptime);
	});

	smoothedDelay.reset(getSampleRate(), PluginConfig::ramptime);
}
