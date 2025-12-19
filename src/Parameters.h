#pragma once

#include <JuceHeader.h>
#include "Types.h"

struct ParameterSettings {
	const String name;
	const float min;
	const float max;
	const float defaultVal;
};

namespace PluginConfig {
	static const ParameterSettings range { "Range", 0.0f, 50.0f, 0.0f };
	static const ParameterSettings center { "Center", 0.0f, 1.0f,  0.5f };
	static const ParameterSettings speed { "Speed",  1.0f, 16.0f, 1.0f };
}

struct Parameter {
	ParameterSettings settings;
	std::atomic<float>* parameter = nullptr; // Initialize to nullptr!
	SmoothedValue<float> smoothed;

	Parameter(ParameterSettings parameterSettings)
	: settings(parameterSettings) {}

	void link(APVTS& apvts, double sampleRate) {
		parameter = apvts.getRawParameterValue(settings.name);
		if (parameter) {
			smoothed.reset(sampleRate, 0.05);
			smoothed.setCurrentAndTargetValue(parameter->load());
		}
	}
};

struct Parameters {
	// Initialize with hardcoded values immediately
	Parameter range { PluginConfig::range };
	Parameter center { PluginConfig::center };
	Parameter speed { PluginConfig::speed };

	// Default constructor is fine now
	Parameters() {}

	void forEach(std::function<void(Parameter&)> callback) {
		callback(range);
		callback(center);
		callback(speed);
	}
};
