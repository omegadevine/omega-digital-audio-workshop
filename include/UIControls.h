#ifndef UICONTROLS_H
#define UICONTROLS_H

#include "UIWindow.h"
#include <functional>

namespace OmegaDAW {

class UIButton : public UIComponent {
private:
    std::string label;
    std::function<void()> onClick;
    bool pressed;
    
public:
    UIButton(const std::string& id, const std::string& label);
    
    void render() override;
    void handleMouseDown(float x, float y) override;
    void handleMouseUp(float x, float y) override;
    
    void setLabel(const std::string& label) { this->label = label; }
    void setOnClick(std::function<void()> callback) { onClick = callback; }
};

class UISlider : public UIComponent {
private:
    float value;
    float minValue, maxValue;
    bool horizontal;
    bool dragging;
    std::function<void(float)> onValueChanged;
    
public:
    UISlider(const std::string& id, float min = 0.0f, float max = 1.0f, bool horizontal = true);
    
    void render() override;
    void handleMouseDown(float x, float y) override;
    void handleMouseUp(float x, float y) override;
    void handleMouseMove(float x, float y) override;
    
    void setValue(float v);
    float getValue() const { return value; }
    void setOnValueChanged(std::function<void(float)> callback) { onValueChanged = callback; }
};

class UIKnob : public UIComponent {
private:
    float value;
    float minValue, maxValue;
    float startAngle, endAngle;
    bool dragging;
    float dragStartY;
    std::function<void(float)> onValueChanged;
    
public:
    UIKnob(const std::string& id, float min = 0.0f, float max = 1.0f);
    
    void render() override;
    void handleMouseDown(float x, float y) override;
    void handleMouseUp(float x, float y) override;
    void handleMouseMove(float x, float y) override;
    
    void setValue(float v);
    float getValue() const { return value; }
    void setOnValueChanged(std::function<void(float)> callback) { onValueChanged = callback; }
};

class UILabel : public UIComponent {
private:
    std::string text;
    float fontSize;
    
public:
    UILabel(const std::string& id, const std::string& text);
    
    void render() override;
    void setText(const std::string& text) { this->text = text; }
    void setFontSize(float size) { fontSize = size; }
};

class UITextBox : public UIComponent {
private:
    std::string text;
    std::string placeholder;
    bool focused;
    int cursorPosition;
    
public:
    UITextBox(const std::string& id);
    
    void render() override;
    void handleMouseDown(float x, float y) override;
    void handleKeyDown(int key) override;
    
    void setText(const std::string& text) { this->text = text; }
    const std::string& getText() const { return text; }
    void setPlaceholder(const std::string& ph) { placeholder = ph; }
};

class UIToggle : public UIComponent {
private:
    bool checked;
    std::string label;
    std::function<void(bool)> onToggled;
    
public:
    UIToggle(const std::string& id, const std::string& label);
    
    void render() override;
    void handleMouseDown(float x, float y) override;
    
    void setChecked(bool c);
    bool isChecked() const { return checked; }
    void setOnToggled(std::function<void(bool)> callback) { onToggled = callback; }
};

}

#endif
