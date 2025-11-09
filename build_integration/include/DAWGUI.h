#ifndef DAW_GUI_H
#define DAW_GUI_H

#include "DAWApplication.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <memory>

namespace OmegaDAW {

// GUI Color scheme
struct GUIColors {
    SDL_Color background = {30, 30, 35, 255};
    SDL_Color panel = {40, 40, 45, 255};
    SDL_Color border = {60, 60, 70, 255};
    SDL_Color text = {220, 220, 220, 255};
    SDL_Color accent = {100, 180, 255, 255};
    SDL_Color button = {50, 50, 55, 255};
    SDL_Color buttonHover = {70, 70, 75, 255};
    SDL_Color buttonActive = {100, 180, 255, 255};
    SDL_Color waveform = {100, 255, 100, 255};
    SDL_Color meter = {255, 200, 50, 255};
};

// GUI components
struct Button {
    std::string label;
    SDL_Rect rect;
    bool hovered = false;
    bool pressed = false;
};

struct Fader {
    SDL_Rect rect;
    float value = 0.75f; // 0.0 to 1.0
    bool dragging = false;
    std::string label;
};

struct Meter {
    SDL_Rect rect;
    float level = 0.0f; // 0.0 to 1.0
    float peak = 0.0f;
};

class DAWGUI {
public:
    DAWGUI(DAWApplication* app);
    ~DAWGUI();
    
    bool initialize(int width, int height);
    void shutdown();
    
    void processEvents();
    void render();
    
    bool shouldQuit() const { return quit; }
    
private:
    void renderTransportPanel();
    void renderMixerPanel();
    void renderTimelinePanel();
    void renderMenuBar();
    void renderStatusBar();
    
    void drawButton(const Button& button);
    void drawFader(const Fader& fader);
    void drawMeter(const Meter& meter);
    void drawText(const std::string& text, int x, int y, SDL_Color color);
    void drawRect(const SDL_Rect& rect, SDL_Color color, bool filled = true);
    void drawLine(int x1, int y1, int x2, int y2, SDL_Color color);
    
    void handleMouseDown(int x, int y);
    void handleMouseUp(int x, int y);
    void handleMouseMove(int x, int y);
    void handleKeyDown(SDL_Keycode key);
    
    bool isPointInRect(int x, int y, const SDL_Rect& rect);
    
    DAWApplication* daw;
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    
    GUIColors colors;
    
    // GUI Components
    std::vector<Button> transportButtons;
    std::vector<Fader> channelFaders;
    std::vector<Meter> channelMeters;
    
    Button playButton;
    Button stopButton;
    Button recordButton;
    Button pauseButton;
    
    // State
    bool quit;
    int windowWidth;
    int windowHeight;
    int mouseX, mouseY;
};

} // namespace OmegaDAW

#endif // DAW_GUI_H
