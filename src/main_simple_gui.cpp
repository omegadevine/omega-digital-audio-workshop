#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

// Simple color scheme
struct Color {
    SDL_Color background = {30, 30, 35, 255};
    SDL_Color panel = {40, 40, 45, 255};
    SDL_Color border = {60, 60, 70, 255};
    SDL_Color text = {220, 220, 220, 255};
    SDL_Color accent = {100, 180, 255, 255};
    SDL_Color button = {50, 50, 55, 255};
    SDL_Color buttonHover = {70, 70, 75, 255};
    SDL_Color buttonActive = {100, 180, 255, 255};
    SDL_Color meter = {100, 255, 100, 255};
} colors;

// Simple button
struct Button {
    std::string label;
    SDL_Rect rect;
    bool hovered = false;
    bool pressed = false;
};

// Simple fader
struct Fader {
    SDL_Rect rect;
    float value = 0.75f;
    bool dragging = false;
    std::string label;
};

// Simple meter
struct Meter {
    SDL_Rect rect;
    float level = 0.0f;
    float peak = 0.0f;
};

// Application state
struct AppState {
    bool quit = false;
    bool playing = false;
    bool recording = false;
    double position = 0.0;
    double tempo = 120.0;
    float cpuLoad = 0.0f;
} state;

// Forward declarations
void drawButton(SDL_Renderer* renderer, TTF_Font* font, const Button& button);
void drawFader(SDL_Renderer* renderer, TTF_Font* font, const Fader& fader);
void drawMeter(SDL_Renderer* renderer, const Meter& meter);
void drawText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color);
void drawRect(SDL_Renderer* renderer, const SDL_Rect& rect, SDL_Color color, bool filled = true);
bool isPointInRect(int x, int y, const SDL_Rect& rect);

int main(int argc, char* argv[]) {
    std::cout << "=== Omega Digital Audio Workshop GUI ===" << std::endl;
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    int windowWidth = 1280;
    int windowHeight = 720;
    
    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "Omega DAW",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        windowWidth, windowHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    
    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Initialize SDL_ttf
    if (TTF_Init() < 0) {
        std::cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
    }
    
    // Load font
    TTF_Font* font = nullptr;
    const char* fontPaths[] = {
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        nullptr
    };
    
    for (int i = 0; fontPaths[i] != nullptr; ++i) {
        font = TTF_OpenFont(fontPaths[i], 14);
        if (font) {
            std::cout << "Loaded font: " << fontPaths[i] << std::endl;
            break;
        }
    }
    
    if (!font) {
        std::cerr << "Warning: Failed to load font. Labels will not be visible." << std::endl;
    }
    
    // Setup UI components
    Button playButton = {"Play", {20, windowHeight - 60, 80, 40}, false, false};
    Button stopButton = {"Stop", {110, windowHeight - 60, 80, 40}, false, false};
    Button recordButton = {"Rec", {200, windowHeight - 60, 80, 40}, false, false};
    Button pauseButton = {"Pause", {290, windowHeight - 60, 80, 40}, false, false};
    
    std::vector<Fader> faders;
    std::vector<Meter> meters;
    
    for (int i = 0; i < 8; ++i) {
        Fader fader;
        fader.rect = {windowWidth - 200 + i * 25, 100, 20, 200};
        fader.label = "Ch" + std::to_string(i + 1);
        fader.value = 0.75f;
        faders.push_back(fader);
        
        Meter meter;
        meter.rect = {windowWidth - 200 + i * 25, 50, 20, 30};
        meters.push_back(meter);
    }
    
    std::cout << "GUI initialized successfully" << std::endl;
    std::cout << "\nControls:" << std::endl;
    std::cout << "  SPACE - Play/Stop" << std::endl;
    std::cout << "  R     - Record" << std::endl;
    std::cout << "  ESC   - Quit" << std::endl;
    std::cout << "\nPress buttons or drag faders!" << std::endl;
    
    // Main loop
    auto lastTime = std::chrono::high_resolution_clock::now();
    int frameCount = 0;
    double fpsTimer = 0.0;
    
    while (!state.quit) {
        auto frameStart = std::chrono::high_resolution_clock::now();
        
        // Process events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    state.quit = true;
                    break;
                    
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        int x = event.button.x;
                        int y = event.button.y;
                        
                        // Check buttons
                        if (isPointInRect(x, y, playButton.rect)) {
                            playButton.pressed = true;
                            state.playing = !state.playing;
                            std::cout << (state.playing ? "Playing" : "Stopped") << std::endl;
                        } else if (isPointInRect(x, y, stopButton.rect)) {
                            stopButton.pressed = true;
                            state.playing = false;
                            state.position = 0.0;
                            std::cout << "Stopped" << std::endl;
                        } else if (isPointInRect(x, y, recordButton.rect)) {
                            recordButton.pressed = true;
                            state.recording = !state.recording;
                            std::cout << (state.recording ? "Recording" : "Not recording") << std::endl;
                        } else if (isPointInRect(x, y, pauseButton.rect)) {
                            pauseButton.pressed = true;
                            state.playing = false;
                            std::cout << "Paused" << std::endl;
                        }
                        
                        // Check faders
                        for (auto& fader : faders) {
                            if (isPointInRect(x, y, fader.rect)) {
                                fader.dragging = true;
                                fader.value = 1.0f - (float)(y - fader.rect.y) / fader.rect.h;
                                fader.value = std::max(0.0f, std::min(1.0f, fader.value));
                            }
                        }
                    }
                    break;
                    
                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        playButton.pressed = false;
                        stopButton.pressed = false;
                        recordButton.pressed = false;
                        pauseButton.pressed = false;
                        
                        for (auto& fader : faders) {
                            fader.dragging = false;
                        }
                    }
                    break;
                    
                case SDL_MOUSEMOTION: {
                    int x = event.motion.x;
                    int y = event.motion.y;
                    
                    // Update hover states
                    playButton.hovered = isPointInRect(x, y, playButton.rect);
                    stopButton.hovered = isPointInRect(x, y, stopButton.rect);
                    recordButton.hovered = isPointInRect(x, y, recordButton.rect);
                    pauseButton.hovered = isPointInRect(x, y, pauseButton.rect);
                    
                    // Update dragging faders
                    for (auto& fader : faders) {
                        if (fader.dragging) {
                            fader.value = 1.0f - (float)(y - fader.rect.y) / fader.rect.h;
                            fader.value = std::max(0.0f, std::min(1.0f, fader.value));
                        }
                    }
                    break;
                }
                    
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_SPACE:
                            state.playing = !state.playing;
                            std::cout << (state.playing ? "Playing" : "Stopped") << std::endl;
                            break;
                        case SDLK_ESCAPE:
                            state.quit = true;
                            break;
                        case SDLK_r:
                            state.recording = !state.recording;
                            std::cout << (state.recording ? "Recording" : "Not recording") << std::endl;
                            break;
                    }
                    break;
                    
                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        windowWidth = event.window.data1;
                        windowHeight = event.window.data2;
                        
                        // Update button positions
                        playButton.rect.y = windowHeight - 60;
                        stopButton.rect.y = windowHeight - 60;
                        recordButton.rect.y = windowHeight - 60;
                        pauseButton.rect.y = windowHeight - 60;
                    }
                    break;
            }
        }
        
        // Update state
        if (state.playing) {
            auto deltaTime = std::chrono::duration<double>(frameStart - lastTime).count();
            state.position += deltaTime;
            
            // Update meters (simulate audio)
            for (auto& meter : meters) {
                meter.level = 0.3f + (rand() % 40) / 100.0f;
                meter.peak = std::max(meter.peak, meter.level);
            }
        } else {
            // Decay meters
            for (auto& meter : meters) {
                meter.level *= 0.95f;
                meter.peak *= 0.98f;
            }
        }
        
        state.cpuLoad = 0.05f + (rand() % 10) / 100.0f;
        
        // Render
        SDL_SetRenderDrawColor(renderer, colors.background.r, colors.background.g, 
                              colors.background.b, colors.background.a);
        SDL_RenderClear(renderer);
        
        // Menu bar
        SDL_Rect menuBar = {0, 0, windowWidth, 30};
        drawRect(renderer, menuBar, colors.panel);
        drawText(renderer, font, "File  Edit  View  Track  Insert  Help", 10, 8, colors.text);
        
        // Timeline panel
        SDL_Rect timelinePanel = {0, 30, windowWidth - 220, windowHeight - 110};
        drawRect(renderer, timelinePanel, colors.background);
        drawText(renderer, font, "TIMELINE / ARRANGEMENT", 20, 40, colors.accent);
        
        // Grid
        SDL_SetRenderDrawColor(renderer, colors.border.r, colors.border.g, 
                              colors.border.b, colors.border.a);
        for (int x = 0; x < windowWidth - 220; x += 50) {
            SDL_RenderDrawLine(renderer, x, 60, x, windowHeight - 110);
        }
        for (int y = 60; y < windowHeight - 110; y += 50) {
            SDL_RenderDrawLine(renderer, 0, y, windowWidth - 220, y);
        }
        
        // Playhead
        if (state.playing) {
            int playheadX = 20 + (int)(state.position * 20.0) % (windowWidth - 240);
            SDL_SetRenderDrawColor(renderer, colors.accent.r, colors.accent.g, 
                                  colors.accent.b, colors.accent.a);
            SDL_RenderDrawLine(renderer, playheadX, 60, playheadX, windowHeight - 110);
        }
        
        // Example clips
        SDL_Rect clip1 = {50, 100, 150, 40};
        SDL_Rect clip2 = {220, 150, 200, 40};
        drawRect(renderer, clip1, colors.button);
        drawRect(renderer, clip2, colors.button);
        drawText(renderer, font, "Audio Clip 1", 55, 115, colors.text);
        drawText(renderer, font, "MIDI Clip 1", 225, 165, colors.text);
        
        // Mixer panel
        SDL_Rect mixerPanel = {windowWidth - 220, 30, 220, windowHeight - 110};
        drawRect(renderer, mixerPanel, colors.panel);
        drawText(renderer, font, "MIXER", windowWidth - 200, 40, colors.accent);
        
        for (size_t i = 0; i < faders.size(); ++i) {
            drawMeter(renderer, meters[i]);
            drawFader(renderer, font, faders[i]);
            drawText(renderer, font, faders[i].label, faders[i].rect.x - 5, 
                    faders[i].rect.y + faders[i].rect.h + 5, colors.text);
        }
        
        // Transport panel
        SDL_Rect transportPanel = {0, windowHeight - 80, windowWidth, 80};
        drawRect(renderer, transportPanel, colors.panel);
        
        drawButton(renderer, font, playButton);
        drawButton(renderer, font, stopButton);
        drawButton(renderer, font, recordButton);
        drawButton(renderer, font, pauseButton);
        
        // Transport info
        int minutes = (int)(state.position / 60.0);
        int seconds = (int)state.position % 60;
        int ms = (int)((state.position - (int)state.position) * 1000);
        
        char timeStr[64];
        sprintf(timeStr, "Time: %02d:%02d.%03d", minutes, seconds, ms);
        drawText(renderer, font, timeStr, 400, windowHeight - 40, colors.text);
        
        char tempoStr[64];
        sprintf(tempoStr, "BPM: %.1f", state.tempo);
        drawText(renderer, font, tempoStr, 600, windowHeight - 40, colors.text);
        
        // Status
        std::string status = "Ready";
        if (state.playing) status = "Playing";
        if (state.recording) status = "Recording";
        drawText(renderer, font, "Status: " + status, 800, windowHeight - 40, colors.text);
        
        char cpuStr[64];
        sprintf(cpuStr, "CPU: %.0f%%", state.cpuLoad * 100);
        drawText(renderer, font, cpuStr, 950, windowHeight - 40, colors.text);
        
        // Present
        SDL_RenderPresent(renderer);
        
        // FPS counter
        frameCount++;
        auto frameEnd = std::chrono::high_resolution_clock::now();
        double frameDuration = std::chrono::duration<double>(frameEnd - frameStart).count();
        fpsTimer += frameDuration;
        
        if (fpsTimer >= 1.0) {
            std::cout << "FPS: " << frameCount << std::endl;
            frameCount = 0;
            fpsTimer = 0.0;
        }
        
        lastTime = frameStart;
    }
    
    std::cout << "\nShutting down..." << std::endl;
    
    if (font) {
        TTF_CloseFont(font);
    }
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    std::cout << "Goodbye!" << std::endl;
    return 0;
}

// Helper functions
void drawButton(SDL_Renderer* renderer, TTF_Font* font, const Button& button) {
    SDL_Color bgColor = colors.button;
    if (button.pressed) {
        bgColor = colors.buttonActive;
    } else if (button.hovered) {
        bgColor = colors.buttonHover;
    }
    
    drawRect(renderer, button.rect, bgColor);
    drawRect(renderer, button.rect, colors.border, false);
    
    int textX = button.rect.x + (button.rect.w - button.label.length() * 6) / 2;
    int textY = button.rect.y + (button.rect.h - 12) / 2;
    drawText(renderer, font, button.label, textX, textY, colors.text);
}

void drawFader(SDL_Renderer* renderer, TTF_Font* font, const Fader& fader) {
    drawRect(renderer, fader.rect, colors.border);
    
    int thumbY = fader.rect.y + (int)((1.0f - fader.value) * fader.rect.h);
    SDL_Rect thumb = {fader.rect.x - 2, thumbY - 5, fader.rect.w + 4, 10};
    drawRect(renderer, thumb, fader.dragging ? colors.accent : colors.button);
}

void drawMeter(SDL_Renderer* renderer, const Meter& meter) {
    drawRect(renderer, meter.rect, colors.border);
    
    int levelHeight = (int)(meter.level * meter.rect.h);
    SDL_Rect levelRect = {meter.rect.x, meter.rect.y + meter.rect.h - levelHeight,
                          meter.rect.w, levelHeight};
    drawRect(renderer, levelRect, colors.meter);
    
    if (meter.peak > 0.01f) {
        int peakY = meter.rect.y + (int)((1.0f - meter.peak) * meter.rect.h);
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawLine(renderer, meter.rect.x, peakY, 
                          meter.rect.x + meter.rect.w, peakY);
    }
}

void drawText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color) {
    if (!font || text.empty()) {
        // Fallback: draw a rectangle where text would be
        SDL_Rect textRect = {x, y, (int)text.length() * 8, 14};
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderDrawRect(renderer, &textRect);
        return;
    }
    
    // Render text with SDL_ttf
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!textSurface) {
        return;
    }
    
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        SDL_FreeSurface(textSurface);
        return;
    }
    
    SDL_Rect destRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, nullptr, &destRect);
    
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

void drawRect(SDL_Renderer* renderer, const SDL_Rect& rect, SDL_Color color, bool filled) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    if (filled) {
        SDL_RenderFillRect(renderer, &rect);
    } else {
        SDL_RenderDrawRect(renderer, &rect);
    }
}

bool isPointInRect(int x, int y, const SDL_Rect& rect) {
    return x >= rect.x && x < rect.x + rect.w &&
           y >= rect.y && y < rect.y + rect.h;
}
