#pragma once

#include <JuceHeader.h>
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

public:
	int maxWidth, maxHeight;

	KnobWithEditor(AudioProcessorValueTreeState& apvts, const String& paramID, int maxWidth, int maxHeight)
		: attachment(apvts, paramID, slider) 
	{
		if (auto* param = apvts.getParameter(paramID))
			displayName = param->getName(100);

		this->maxWidth = maxWidth;
		this->maxHeight = maxHeight;

		setInterceptsMouseClicks(false, true);

		lookAndFeel = std::make_unique<ModernLookAndFeel>();
		slider.setLookAndFeel(lookAndFeel.get());
		slider.setSliderStyle(Slider::RotaryVerticalDrag);
		slider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

		slider.addMouseListener(this, false);

		editor.setJustification(Justification::centred);
		editor.setColour(TextEditor::backgroundColourId, Colours::transparentBlack);
		editor.setColour(TextEditor::outlineColourId, Colours::transparentBlack);
		editor.setColour(TextEditor::focusedOutlineColourId, Colours::transparentBlack);
		editor.setColour(TextEditor::textColourId, Colours::white);
		
		editor.setInterceptsMouseClicks(false, false);

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
			editor.setCaretVisible(false);
		};

		slider.onDragEnd = [this] {
			editor.setCaretVisible(true);
		};

		addAndMakeVisible(slider);
		addAndMakeVisible(editor);
	}

	bool shouldShowValue() const {
		return slider.isMouseOver() || slider.isMouseButtonDown() || editor.hasKeyboardFocus(true);
	}

	void mouseEnter(const MouseEvent&) override { repaint(); }
	void mouseExit(const MouseEvent&) override { repaint(); }
	
	void mouseDown(const MouseEvent& e) override {
		if (shouldShowValue() && !slider.isMouseButtonDown()) {
			editor.moveCaretToEndOfLine(false);
			editor.setInterceptsMouseClicks(true, true);
			editor.grabKeyboardFocus();
		}
	}

	void paintOverChildren(Graphics& g) override {
		auto bounds = getModifiedBounds();
		bool showValue = shouldShowValue();

		editor.setAlpha(showValue ? 1.0f : 0.0f);
		
		bool isTyping = editor.hasKeyboardFocus(true);
		editor.setInterceptsMouseClicks(isTyping, isTyping);

		if (!showValue) {
			float fontSize = jmin(bounds.getHeight(), bounds.getWidth()) * 0.17f;
			g.setColour(Colours::white.withAlpha(0.9f));
			g.setFont(FontOptions(fontSize).withStyle("Regular"));
			g.drawText(displayName, bounds, Justification::centred);
		}
	}

	Rectangle<int> getModifiedBounds() {
		return getLocalBounds().withSizeKeepingCentre(jmin(getWidth(), maxWidth), jmin(getHeight(), maxHeight));
	}

	void updateEditorBounds() {
		auto bounds = getModifiedBounds();
		Font font = FontOptions(jmin(bounds.getHeight(), bounds.getWidth()) * 0.2f);
		int textWidth = font.getStringWidth(editor.getText()) + 20;
		editor.applyFontToAllText(font);
		editor.setBounds(bounds.withSizeKeepingCentre(textWidth, font.getHeight()));
	}

	void resized() override {
		slider.setBounds(getModifiedBounds());
		updateEditorBounds();
	}
};
