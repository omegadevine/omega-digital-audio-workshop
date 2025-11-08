#include "UIWindow.h"
#include <iostream>
#include <algorithm>

namespace OmegaDAW {

UIComponent::UIComponent(const std::string& id)
    : id(id), visible(true), enabled(true),
      backgroundColor(0.2f, 0.2f, 0.2f, 1.0f),
      foregroundColor(1.0f, 1.0f, 1.0f, 1.0f) {
}

UIWindow::UIWindow(const std::string& title, int width, int height)
    : title(title), width(width), height(height), 
      nativeWindow(nullptr), running(false),
      backgroundColor(0.15f, 0.15f, 0.15f, 1.0f) {
}

UIWindow::~UIWindow() {
    shutdown();
}

bool UIWindow::initialize() {
    // Platform-specific window creation would go here
    // For now, this is a stub implementation
    std::cout << "Initializing UI Window: " << title << " (" << width << "x" << height << ")" << std::endl;
    running = true;
    return true;
}

void UIWindow::shutdown() {
    if (nativeWindow) {
        // Platform-specific cleanup
        nativeWindow = nullptr;
    }
    components.clear();
    running = false;
}

void UIWindow::show() {
    // Platform-specific show window
    std::cout << "Showing window: " << title << std::endl;
}

void UIWindow::hide() {
    // Platform-specific hide window
    std::cout << "Hiding window: " << title << std::endl;
}

void UIWindow::setTitle(const std::string& newTitle) {
    title = newTitle;
    // Platform-specific title update
}

void UIWindow::addComponent(std::shared_ptr<UIComponent> component) {
    if (component) {
        components.push_back(component);
    }
}

void UIWindow::removeComponent(const std::string& id) {
    components.erase(
        std::remove_if(components.begin(), components.end(),
            [&id](const std::shared_ptr<UIComponent>& comp) {
                return comp->getId() == id;
            }),
        components.end()
    );
}

std::shared_ptr<UIComponent> UIWindow::getComponent(const std::string& id) {
    auto it = std::find_if(components.begin(), components.end(),
        [&id](const std::shared_ptr<UIComponent>& comp) {
            return comp->getId() == id;
        });
    
    return (it != components.end()) ? *it : nullptr;
}

void UIWindow::processEvents() {
    // Platform-specific event processing
    // This would poll/process window events and dispatch to components
}

void UIWindow::render() {
    // Clear background
    // Platform-specific rendering would go here
    
    // Render all visible components
    for (auto& component : components) {
        if (component && component->isVisible()) {
            component->render();
        }
    }
}

}
