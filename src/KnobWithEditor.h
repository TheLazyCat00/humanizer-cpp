#pragma once

#include <JuceHeader.h>
#include "Types.h"
#include "LookAndFeel.h"

class KnobWithEditor : public Component {
	void commitEditorValue() {
		auto v = editor.getText().getDoubleValue();
		slider.setValue(v, sendNotificationSync);
	}

	std::unique_ptr<ModernLookAndFeel> lookAndFeel;
	Slider slider;
	TextEditor editor;
	APVTS::SliderAttachment attachment;
public:
	int maxWidth;
	int maxHeight;
	KnobWithEditor(
			AudioProcessorValueTreeState& apvts,
			const String& paramID,
			int maxWidth,
			int maxHeight)
	: attachment(apvts, paramID, slider) {
		this->maxWidth = maxWidth;
		this->maxHeight = maxHeight;
		lookAndFeel = std::make_unique<ModernLookAndFeel>();
		slider.setLookAndFeel(lookAndFeel.get());
		slider.setSliderStyle(Slider::RotaryVerticalDrag);
		slider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

		editor.setJustification(Justification::centred);
		editor.setInputRestrictions(0, "0123456789.");
		editor.setColour(TextEditor::backgroundColourId, Colours::transparentBlack);
		editor.setColour(TextEditor::outlineColourId, Colours::transparentBlack);
		editor.setColour(TextEditor::focusedOutlineColourId, Colours::transparentBlack);

		editor.onReturnKey = [this] { commitEditorValue(); };
		editor.onFocusLost = [this] { commitEditorValue(); };

		editor.setText(
			String(slider.getValue(), 1),
			dontSendNotification);

		slider.onDragStart = [this] {
			editor.moveCaretToEndOfLine(false);
			editor.unfocusAllComponents();
		};

		slider.onValueChange = [this] {
			editor.setText(
				String(slider.getValue(), 1),
				dontSendNotification);
			updateEditorBounds();
		};

		addAndMakeVisible(slider);
		addAndMakeVisible(editor);

	}

	Rectangle<int> getModifiedBounds() {
		auto bounds = getLocalBounds();
		
		auto newBounds = bounds.withSizeKeepingCentre(
			jmin(bounds.getWidth(), maxWidth), 
			jmin(bounds.getHeight(), maxHeight)
		);

		return newBounds;
	}

	void updateEditorBounds() {
		auto bounds = getModifiedBounds();

		float fontSize = jmin(bounds.getHeight(), bounds.getWidth()) * 0.2f;
		Font font = FontOptions(fontSize);
		
		// 2. Measure the text
		auto text = editor.getText();
		int textWidth = font.getStringWidth(text) + 4;
		int textHeight = font.getHeight();

		editor.applyFontToAllText(font);

		editor.setBounds(bounds.withSizeKeepingCentre(textWidth, textHeight));
	}

	void resized() override {
		slider.setBounds(getModifiedBounds());
		updateEditorBounds();
	}
};
