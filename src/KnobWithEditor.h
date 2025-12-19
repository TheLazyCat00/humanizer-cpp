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
			updateEditorBounds();
		};
	}

	void updateEditorBounds() {
		auto bounds = getLocalBounds();
		
		float fontSize = jmin(bounds.getHeight(), bounds.getWidth()) * 0.14f;
		Font font = FontOptions(fontSize, Font::bold);
		
		// 2. Measure the text
		auto text = editor.getText();
		int textWidth = font.getStringWidth(text) + 4;
		int textHeight = font.getHeight();

		editor.applyFontToAllText(font);

		editor.setBounds(bounds.withSizeKeepingCentre(textWidth, textHeight));
	}

	void resized() override {
		slider.setBounds(getLocalBounds());
		updateEditorBounds();
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
