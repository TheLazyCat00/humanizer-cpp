#pragma once

#include <JuceHeader.h>
#include <functional>
#include <atomic>
#include "Types.h"

struct ParameterSettings {
	const String& name;
	const float min;
	const float max;
	const float defaultVal;
	const String& desc;
};

namespace PluginConfig {
	static const ParameterSettings range {
		"Range",
		0.0f,
		200.0f,
		0.0f,
		"Sets the amount of ms the delay can go up + down (in total). Do not automate this parameter."
	};
	static const ParameterSettings center {
		"Center",
		-1.0f,
		1.0f,
		0.0f,
		"Sets the center of the LFO. Do not automate this parameter."
	};
	static const ParameterSettings speed {
		"Speed",
		1.0f,
		16.0f,
		1.0f,
		"Sets the time for one slope to finish in beats. Feel free to automate this parameter."
	};
	static const float ramptime = 0.05;
}

struct Parameter {
	ParameterSettings settings;
	std::atomic<float>* parameter = nullptr;
	SmoothedValue<float> smoothed;

	Parameter(ParameterSettings parameterSettings)
	: settings(parameterSettings) {}

	void link(APVTS& apvts, double sampleRate) {
		parameter = apvts.getRawParameterValue(settings.name);
		if (parameter) {
			smoothed.reset(sampleRate, PluginConfig::ramptime);
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
