#pragma once

#include <JuceHeader.h>
#include "Types.h"
#include "LookAndFeel.h"

class KnobWithEditor : public Component {
	void commitEditorValue() {
		auto v = editor.getText().getDoubleValue();
		slider.setValue(v, sendNotificationSync);
		
		editor.setVisible(false);
		repaint();
	}

	std::unique_ptr<ModernLookAndFeel> lookAndFeel;
	Slider slider;
	TextEditor editor;
	APVTS::SliderAttachment attachment;
	String displayName;

public:
	int maxWidth;
	int maxHeight;

	KnobWithEditor(
			AudioProcessorValueTreeState& apvts,
			const String& paramID,
			int maxWidth,
			int maxHeight)
	: attachment(apvts, paramID, slider) {
		
		if (auto* param = apvts.getParameter(paramID))
			displayName = param->getName(100);

		this->maxWidth = maxWidth;
		this->maxHeight = maxHeight;

		setInterceptsMouseClicks(false, true);

		lookAndFeel = std::make_unique<ModernLookAndFeel>();
		slider.setLookAndFeel(lookAndFeel.get());
		slider.setSliderStyle(Slider::RotaryVerticalDrag);
		slider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

		// Listen to the slider because the parent ignores mouse events now!
		slider.addMouseListener(this, false);
		editor.addMouseListener(this, false);

		editor.setJustification(Justification::centred);
		editor.setInputRestrictions(0, "0123456789.");
		editor.setColour(TextEditor::backgroundColourId, Colours::transparentBlack);
		editor.setColour(TextEditor::outlineColourId, Colours::transparentBlack);
		editor.setColour(TextEditor::focusedOutlineColourId, Colours::transparentBlack);
		editor.setColour(TextEditor::textColourId, Colours::white);

		editor.onReturnKey = [this] { commitEditorValue(); };
		editor.onFocusLost = [this] { commitEditorValue(); };

		editor.setText(String(slider.getValue(), 1), dontSendNotification);
		editor.setVisible(false);

		slider.onDragStart = [this] {
			editor.unfocusAllComponents();
			editor.setVisible(true); 
		};

		slider.onValueChange = [this] {
			editor.setText(String(slider.getValue(), 1), dontSendNotification);
			updateEditorBounds();
			repaint(); 
		};

		addAndMakeVisible(slider);
		addChildComponent(editor);
	}

	void mouseEnter(const MouseEvent& e) override {
		// (Double check to prevent background firing, though setIntercepts fixes most of it)
		if (e.eventComponent == &slider || e.eventComponent == &editor) {
			editor.setVisible(true);
			repaint();
		}
	}

	void mouseExit(const MouseEvent& e) override {
		if (!(slider.isMouseOver() || editor.isMouseOver()) && !editor.hasKeyboardFocus(true)) {
			editor.setVisible(false);
			repaint();
		}
	}

	void paintOverChildren(Graphics& g) override {
		if (!editor.isVisible()) {
			auto bounds = getModifiedBounds();
			float fontSize = jmin(bounds.getHeight(), bounds.getWidth()) * 0.17f;

			g.setColour(Colours::white.withAlpha(0.9f));
			g.setFont(FontOptions(fontSize).withStyle("Regular"));
			g.drawText(displayName, bounds, Justification::centred);
		}
	}

	Rectangle<int> getModifiedBounds() {
		auto bounds = getLocalBounds();
		return bounds.withSizeKeepingCentre(
			jmin(bounds.getWidth(), maxWidth), 
			jmin(bounds.getHeight(), maxHeight)
		);
	}

	void updateEditorBounds() {
		auto bounds = getModifiedBounds();
		float fontSize = jmin(bounds.getHeight(), bounds.getWidth()) * 0.2f;
		Font font = FontOptions(fontSize);
		
		auto text = editor.getText();
		int textWidth = font.getStringWidth(text) + 20;
		int textHeight = font.getHeight();

		editor.applyFontToAllText(font);
		editor.setBounds(bounds.withSizeKeepingCentre(textWidth, textHeight));
	}

	void resized() override {
		slider.setBounds(getModifiedBounds());
		updateEditorBounds();
	}
};
