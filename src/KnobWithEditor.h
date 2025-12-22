#pragma once

#include <JuceHeader.h>
#include <memory.h>
#include "Types.h"
#include "LookAndFeel.h"

class KnobWithEditor : public Component {
	void commitEditorValue() {
		auto v = editor.getText().getDoubleValue();
		slider.setValue(v, sendNotificationSync);
		editor.giveAwayKeyboardFocus();
		repaint();
	}

	std::unique_ptr<ModernLookAndFeel> lookAndFeel;
	Slider slider;
	TextEditor editor;
	APVTS::SliderAttachment attachment;
	String displayName;
	bool dragging = false;
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KnobWithEditor)
public:
	int maxWidth, maxHeight;

	KnobWithEditor(AudioProcessorValueTreeState& apvts, const String& name, int maxWidth, int maxHeight)
			: attachment(apvts, name, slider) {
		displayName = name;

		this->maxWidth = maxWidth;
		this->maxHeight = maxHeight;

		lookAndFeel = std::make_unique<ModernLookAndFeel>();
		slider.setLookAndFeel(lookAndFeel.get());
		slider.setSliderStyle(Slider::RotaryVerticalDrag);
		slider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

		editor.setJustification(Justification::centred);
		editor.setColour(TextEditor::backgroundColourId, Colours::transparentBlack);
		editor.setColour(TextEditor::outlineColourId, Colours::transparentBlack);
		editor.setColour(TextEditor::focusedOutlineColourId, Colours::transparentBlack);
		editor.setColour(TextEditor::textColourId, Colours::white);
		
		setInterceptsMouseClicks(false, true);
		editor.setInterceptsMouseClicks(true, true);

		editor.onReturnKey = [this] { commitEditorValue(); };
		editor.onFocusLost = [this] { commitEditorValue(); };

		editor.setText(String(slider.getValue(), 1), dontSendNotification);
		editor.setAlpha(0.0f); 

		slider.onValueChange = [this] {
			editor.setText(String(slider.getValue(), 1), dontSendNotification);

			updateEditorBounds();
			repaint();
		};

		slider.onDragStart = [this] {
			resetEditor();
			dragging = true;
		};

		slider.onDragEnd = [this] {
			dragging = false;
		};

		addAndMakeVisible(slider);
		addAndMakeVisible(editor);
	}

	bool shouldShowValue() const {
		return slider.isMouseOver() || editor.isMouseOver() || dragging;
	}

	void resetEditor() {
		editor.giveAwayKeyboardFocus();
		editor.moveCaretToEndOfLine(false);
	}

	void mouseEnter(const MouseEvent&) override { repaint(); }
	void mouseExit(const MouseEvent&) override { repaint(); }
	
	void paintOverChildren(Graphics& g) override {
		auto bounds = getModifiedBounds();
		bool showValue = shouldShowValue();

		editor.setAlpha(showValue ? 1.0f : 0.0f);

		if (!showValue) {
			resetEditor();

			float fontSize = jmin(bounds.getHeight(), bounds.getWidth()) * 0.17f;
			g.setColour(Colours::white.withAlpha(0.9f));
			g.setFont(FontOptions(fontSize).withStyle("Regular"));
			g.drawText(displayName, bounds, Justification::centred);
		}
	}

	Rectangle<int> getModifiedBounds() {
		auto minWidth = jmin(getWidth(), maxWidth);
		auto minHeight = jmin(getHeight(), maxHeight);

		auto minLength = jmin(minWidth, minHeight);
		return getLocalBounds().withSizeKeepingCentre(minLength, minLength);
	}

	void updateEditorBounds() {
		auto bounds = getModifiedBounds();
		Font font = FontOptions(jmin(bounds.getHeight(), bounds.getWidth()) * 0.2f);
		int textWidth = font.getStringWidth(editor.getText()) + 20;
		editor.applyFontToAllText(font);
		editor.setBounds(bounds.withSizeKeepingCentre(textWidth, int(font.getHeight())));
	}

	void resized() override {
		slider.setBounds(getModifiedBounds());
		updateEditorBounds();
	}
};
