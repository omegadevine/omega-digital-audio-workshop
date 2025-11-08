#ifndef UIWINDOW_H
#define UIWINDOW_H

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace OmegaDAW {

struct Color {
    float r, g, b, a;
    Color(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f)
        : r(r), g(g), b(b), a(a) {}
};

struct Rect {
    float x, y, width, height;
    Rect(float x = 0, float y = 0, float w = 0, float h = 0)
        : x(x), y(y), width(w), height(h) {}
    
    bool contains(float px, float py) const {
        return px >= x && px <= x + width && py >= y && py <= y + height;
    }
};

class UIComponent {
protected:
    Rect bounds;
    Color backgroundColor;
    Color foregroundColor;
    bool visible;
    bool enabled;
    std::string id;
    
public:
    UIComponent(const std::string& id = "");
    virtual ~UIComponent() = default;
    
    virtual void render() = 0;
    virtual void update(float deltaTime) {}
    virtual void handleMouseDown(float x, float y) {}
    virtual void handleMouseUp(float x, float y) {}
    virtual void handleMouseMove(float x, float y) {}
    virtual void handleKeyDown(int key) {}
    virtual void handleKeyUp(int key) {}
    
    void setBounds(const Rect& rect) { bounds = rect; }
    const Rect& getBounds() const { return bounds; }
    void setVisible(bool v) { visible = v; }
    bool isVisible() const { return visible; }
    void setEnabled(bool e) { enabled = e; }
    bool isEnabled() const { return enabled; }
    const std::string& getId() const { return id; }
};

class UIWindow {
private:
    std::string title;
    int width, height;
    void* nativeWindow;
    std::vector<std::shared_ptr<UIComponent>> components;
    Color backgroundColor;
    bool running;
    
public:
    UIWindow(const std::string& title, int width, int height);
    ~UIWindow();
    
    bool initialize();
    void shutdown();
    void show();
    void hide();
    void setTitle(const std::string& title);
    
    void addComponent(std::shared_ptr<UIComponent> component);
    void removeComponent(const std::string& id);
    std::shared_ptr<UIComponent> getComponent(const std::string& id);
    
    void processEvents();
    void render();
    bool isRunning() const { return running; }
    void close() { running = false; }
    
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    void setBackgroundColor(const Color& color) { backgroundColor = color; }
};

}

#endif
