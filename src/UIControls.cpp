#include "UIControls.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace OmegaDAW {

// UIButton Implementation
UIButton::UIButton(const std::string& id, const std::string& label)
    : UIComponent(id), label(label), pressed(false) {
    backgroundColor = Color(0.3f, 0.3f, 0.35f, 1.0f);
    foregroundColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
}

void UIButton::render() {
    // Simple rendering stub - would use actual graphics API
    Color renderColor = pressed ? 
        Color(0.5f, 0.5f, 0.6f, 1.0f) : backgroundColor;
    
    // Draw button rectangle and label
    // Actual rendering would be done with OpenGL/DirectX/etc.
}

void UIButton::handleMouseDown(float x, float y) {
    if (bounds.contains(x, y) && enabled) {
        pressed = true;
    }
}

void UIButton::handleMouseUp(float x, float y) {
    if (pressed && bounds.contains(x, y) && enabled) {
        if (onClick) {
            onClick();
        }
    }
    pressed = false;
}

// UISlider Implementation
UISlider::UISlider(const std::string& id, float min, float max, bool horizontal)
    : UIComponent(id), minValue(min), maxValue(max), 
      horizontal(horizontal), dragging(false), value(min) {
    backgroundColor = Color(0.2f, 0.2f, 0.2f, 1.0f);
    foregroundColor = Color(0.4f, 0.6f, 0.8f, 1.0f);
}

void UISlider::render() {
    // Render slider track and thumb
    float normalizedValue = (value - minValue) / (maxValue - minValue);
    
    // Draw track
    // Draw thumb at position based on normalizedValue
}

void UISlider::handleMouseDown(float x, float y) {
    if (bounds.contains(x, y) && enabled) {
        dragging = true;
        handleMouseMove(x, y);
    }
}

void UISlider::handleMouseUp(float x, float y) {
    dragging = false;
}

void UISlider::handleMouseMove(float x, float y) {
    if (dragging) {
        float newValue;
        if (horizontal) {
            float relativeX = (x - bounds.x) / bounds.width;
            relativeX = std::max(0.0f, std::min(1.0f, relativeX));
            newValue = minValue + relativeX * (maxValue - minValue);
        } else {
            float relativeY = (y - bounds.y) / bounds.height;
            relativeY = std::max(0.0f, std::min(1.0f, relativeY));
            newValue = maxValue - relativeY * (maxValue - minValue);
        }
        setValue(newValue);
    }
}

void UISlider::setValue(float v) {
    value = std::max(minValue, std::min(maxValue, v));
    if (onValueChanged) {
        onValueChanged(value);
    }
}

// UIKnob Implementation
UIKnob::UIKnob(const std::string& id, float min, float max)
    : UIComponent(id), minValue(min), maxValue(max), 
      dragging(false), value(min), dragStartY(0),
      startAngle(-135.0f), endAngle(135.0f) {
    backgroundColor = Color(0.2f, 0.2f, 0.2f, 1.0f);
    foregroundColor = Color(0.4f, 0.6f, 0.8f, 1.0f);
}

void UIKnob::render() {
    // Render circular knob with indicator
    float normalizedValue = (value - minValue) / (maxValue - minValue);
    float angle = startAngle + normalizedValue * (endAngle - startAngle);
    
    // Draw circular knob
    // Draw indicator line from center to edge at angle
}

void UIKnob::handleMouseDown(float x, float y) {
    if (bounds.contains(x, y) && enabled) {
        dragging = true;
        dragStartY = y;
    }
}

void UIKnob::handleMouseUp(float x, float y) {
    dragging = false;
}

void UIKnob::handleMouseMove(float x, float y) {
    if (dragging) {
        float delta = dragStartY - y;
        float sensitivity = 0.005f;
        float normalizedDelta = delta * sensitivity;
        float newValue = value + normalizedDelta * (maxValue - minValue);
        setValue(newValue);
        dragStartY = y;
    }
}

void UIKnob::setValue(float v) {
    value = std::max(minValue, std::min(maxValue, v));
    if (onValueChanged) {
        onValueChanged(value);
    }
}

// UILabel Implementation
UILabel::UILabel(const std::string& id, const std::string& text)
    : UIComponent(id), text(text), fontSize(14.0f) {
    backgroundColor = Color(0, 0, 0, 0); // Transparent
}

void UILabel::render() {
    // Render text at position
    // Would use actual text rendering system
}

// UITextBox Implementation
UITextBox::UITextBox(const std::string& id)
    : UIComponent(id), focused(false), cursorPosition(0) {
    backgroundColor = Color(0.15f, 0.15f, 0.15f, 1.0f);
    foregroundColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
}

void UITextBox::render() {
    // Render text box background
    // Render text content
    // Render cursor if focused
}

void UITextBox::handleMouseDown(float x, float y) {
    focused = bounds.contains(x, y) && enabled;
}

void UITextBox::handleKeyDown(int key) {
    if (!focused || !enabled) return;
    
    // Handle text input and cursor movement
    // This is a simplified implementation
    if (key >= 32 && key <= 126) { // Printable characters
        text.insert(cursorPosition, 1, static_cast<char>(key));
        cursorPosition++;
    } else if (key == 8 && cursorPosition > 0) { // Backspace
        text.erase(cursorPosition - 1, 1);
        cursorPosition--;
    }
}

// UIToggle Implementation
UIToggle::UIToggle(const std::string& id, const std::string& label)
    : UIComponent(id), label(label), checked(false) {
    backgroundColor = Color(0.3f, 0.3f, 0.35f, 1.0f);
    foregroundColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
}

void UIToggle::render() {
    // Render toggle button/checkbox
    Color renderColor = checked ? 
        Color(0.4f, 0.6f, 0.8f, 1.0f) : backgroundColor;
    
    // Draw toggle and label
}

void UIToggle::handleMouseDown(float x, float y) {
    if (bounds.contains(x, y) && enabled) {
        setChecked(!checked);
    }
}

void UIToggle::setChecked(bool c) {
    checked = c;
    if (onToggled) {
        onToggled(checked);
    }
}

}
