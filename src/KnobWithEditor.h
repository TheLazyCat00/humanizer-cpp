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
		editor.setColour(TextEditor::backgroundColourId, Colours::transparentBlack);
		editor.setColour(TextEditor::outlineColourId, Colours::transparentBlack);
		editor.setColour(TextEditor::focusedOutlineColourId, Colours::transparentBlack);

		editor.onReturnKey = [this] { commitEditorValue(); };
		editor.onFocusLost = [this] { commitEditorValue(); };

		slider.onDragStart = [this] {
			editor.unfocusAllComponents();
		};

		slider.onValueChange = [this] {
			editor.setText(
				String(slider.getValue(), 1),
				dontSendNotification);
		};
	}

	void resized() override {
		auto bounds = getLocalBounds();
		slider.setBounds(bounds);

		// Make text area centered and proportional
		auto textWidth  = bounds.getWidth();   // adjust ratio as needed
		auto textHeight = bounds.getHeight(); // adjust ratio as needed

		float fontSize = jmin(textWidth, textHeight) * 0.14f;
		auto textArea = bounds.withSizeKeepingCentre(textWidth * fontSize / 150, textHeight * fontSize / 150);

		editor.setBounds(textArea);
		editor.applyFontToAllText(FontOptions(fontSize, Font::bold));
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
