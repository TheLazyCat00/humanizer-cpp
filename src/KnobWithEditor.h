#pragma once

#include <JuceHeader.h>
#include "Types.h"
#include "LookAndFeel.h"

class KnobWithEditor : public Component {
public:
	KnobWithEditor(
			AudioProcessorValueTreeState& apvts,
			const String& paramID)
	: attachment(apvts, paramID, slider) {
		lookAndFeel = std::make_unique<ModernLookAndFeel>();
		slider.setLookAndFeel(lookAndFeel.get());
		slider.setSliderStyle(Slider::RotaryVerticalDrag);
		slider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

		addAndMakeVisible(slider);
		addAndMakeVisible(editor);

		editor.setJustification(Justification::centred);
		editor.setInputRestrictions(0, "0123456789.");
		editor.setColour(TextEditor::backgroundColourId,
				   Colours::transparentBlack);
		editor.setColour(TextEditor::outlineColourId,
				   Colours::transparentBlack);

		// Editor → Slider
		editor.onReturnKey = [this] { commitEditorValue(); };
		editor.onFocusLost = [this] { commitEditorValue(); };

		// Slider → Editor
		slider.onValueChange = [this] {
			editor.setText(
				String(slider.getValue(), 1),
				dontSendNotification);
		};
	}

	void resized() override {
		slider.setBounds(getLocalBounds());

		auto textArea = getLocalBounds().withSizeKeepingCentre(50, 20);
		editor.setBounds(textArea);
	}

private:
	void commitEditorValue() {
		auto v = editor.getText().getDoubleValue();
		slider.setValue(v, sendNotificationSync);
	}

	std::unique_ptr<ModernLookAndFeel> lookAndFeel;
	Slider slider;
	TextEditor editor;
	APVTS::SliderAttachment attachment;
};
