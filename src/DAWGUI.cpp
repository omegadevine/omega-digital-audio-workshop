#include "DAWGUI.h"
#include <iostream>

namespace OmegaDAW {

DAWGUI::DAWGUI(DAWApplication* app)
    : daw(app), window(nullptr), renderer(nullptr), 
      quit(false), windowWidth(0), windowHeight(0),
      mouseX(0), mouseY(0) {
}

DAWGUI::~DAWGUI() {
    shutdown();
}

bool DAWGUI::initialize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Create window
    window = SDL_CreateWindow(
        "Omega DAW",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Setup transport buttons
    playButton = {"Play", {20, windowHeight - 60, 80, 40}, false, false};
    stopButton = {"Stop", {110, windowHeight - 60, 80, 40}, false, false};
    recordButton = {"Rec", {200, windowHeight - 60, 80, 40}, false, false};
    pauseButton = {"Pause", {290, windowHeight - 60, 80, 40}, false, false};
    
    // Setup channel faders and meters
    for (int i = 0; i < 8; ++i) {
        Fader fader;
        fader.rect = {windowWidth - 200 + i * 25, 100, 20, 200};
        fader.label = "Ch" + std::to_string(i + 1);
        fader.value = 0.75f;
        channelFaders.push_back(fader);
        
        Meter meter;
        meter.rect = {windowWidth - 200 + i * 25, 50, 20, 30};
        channelMeters.push_back(meter);
    }
    
    std::cout << "GUI initialized successfully" << std::endl;
    return true;
}

void DAWGUI::shutdown() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    SDL_Quit();
}

void DAWGUI::processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
                
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    handleMouseDown(event.button.x, event.button.y);
                }
                break;
                
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    handleMouseUp(event.button.x, event.button.y);
                }
                break;
                
            case SDL_MOUSEMOTION:
                mouseX = event.motion.x;
                mouseY = event.motion.y;
                handleMouseMove(mouseX, mouseY);
                break;
                
            case SDL_KEYDOWN:
                handleKeyDown(event.key.keysym.sym);
                break;
                
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    windowWidth = event.window.data1;
                    windowHeight = event.window.data2;
                }
                break;
        }
    }
}

void DAWGUI::render() {
    // Clear screen
    SDL_SetRenderDrawColor(renderer, colors.background.r, colors.background.g, 
                          colors.background.b, colors.background.a);
    SDL_RenderClear(renderer);
    
    // Render panels
    renderMenuBar();
    renderTimelinePanel();
    renderMixerPanel();
    renderTransportPanel();
    renderStatusBar();
    
    // Present
    SDL_RenderPresent(renderer);
}

void DAWGUI::renderMenuBar() {
    SDL_Rect menuBar = {0, 0, windowWidth, 30};
    drawRect(menuBar, colors.panel);
    drawText("File  Edit  View  Track  Insert  Help", 10, 8, colors.text);
}

void DAWGUI::renderTransportPanel() {
    // Transport panel background
    SDL_Rect transportPanel = {0, windowHeight - 80, windowWidth, 80};
    drawRect(transportPanel, colors.panel);
    
    // Draw transport buttons
    drawButton(playButton);
    drawButton(stopButton);
    drawButton(recordButton);
    drawButton(pauseButton);
    
    // Draw transport info
    std::string timeStr = "00:00:00.000";
    if (daw && daw->getTransport()) {
        double pos = daw->getTransport()->getPosition();
        int minutes = (int)(pos / 60.0);
        int seconds = (int)pos % 60;
        int ms = (int)((pos - (int)pos) * 1000);
        timeStr = std::to_string(minutes) + ":" + 
                  (seconds < 10 ? "0" : "") + std::to_string(seconds) + "." +
                  (ms < 100 ? "0" : "") + (ms < 10 ? "0" : "") + std::to_string(ms);
    }
    
    drawText("Time: " + timeStr, 400, windowHeight - 40, colors.text);
    
    std::string tempoStr = "BPM: 120.0";
    drawText(tempoStr, 600, windowHeight - 40, colors.text);
}

void DAWGUI::renderMixerPanel() {
    // Mixer panel background
    SDL_Rect mixerPanel = {windowWidth - 220, 30, 220, windowHeight - 110};
    drawRect(mixerPanel, colors.panel);
    
    drawText("MIXER", windowWidth - 200, 40, colors.accent);
    
    // Draw channel strips
    for (size_t i = 0; i < channelFaders.size(); ++i) {
        // Update meter levels (simulate)
        if (daw && daw->isPlaying()) {
            channelMeters[i].level = 0.3f + (rand() % 40) / 100.0f;
            channelMeters[i].peak = std::max(channelMeters[i].peak, channelMeters[i].level);
        } else {
            channelMeters[i].level *= 0.95f;
            channelMeters[i].peak *= 0.98f;
        }
        
        drawMeter(channelMeters[i]);
        drawFader(channelFaders[i]);
        drawText(channelFaders[i].label, channelFaders[i].rect.x - 5, 
                channelFaders[i].rect.y + channelFaders[i].rect.h + 5, colors.text);
    }
}

void DAWGUI::renderTimelinePanel() {
    // Timeline panel background
    SDL_Rect timelinePanel = {0, 30, windowWidth - 220, windowHeight - 110};
    drawRect(timelinePanel, colors.background);
    
    drawText("TIMELINE / ARRANGEMENT", 20, 40, colors.accent);
    
    // Draw timeline grid
    int gridSpacing = 50;
    SDL_SetRenderDrawColor(renderer, colors.border.r, colors.border.g, 
                          colors.border.b, colors.border.a);
    
    for (int x = 0; x < windowWidth - 220; x += gridSpacing) {
        SDL_RenderDrawLine(renderer, x, 60, x, windowHeight - 110);
    }
    
    for (int y = 60; y < windowHeight - 110; y += gridSpacing) {
        SDL_RenderDrawLine(renderer, 0, y, windowWidth - 220, y);
    }
    
    // Draw playhead
    if (daw && daw->getTransport()) {
        double pos = daw->getTransport()->getPosition();
        int playheadX = 20 + (int)(pos * 20.0) % (windowWidth - 240);
        SDL_SetRenderDrawColor(renderer, colors.accent.r, colors.accent.g, 
                              colors.accent.b, colors.accent.a);
        SDL_RenderDrawLine(renderer, playheadX, 60, playheadX, windowHeight - 110);
    }
    
    // Draw some example tracks/clips
    SDL_Rect clip1 = {50, 100, 150, 40};
    SDL_Rect clip2 = {220, 150, 200, 40};
    drawRect(clip1, colors.button);
    drawRect(clip2, colors.button);
    drawText("Audio Clip 1", 55, 115, colors.text);
    drawText("MIDI Clip 1", 225, 165, colors.text);
}

void DAWGUI::renderStatusBar() {
    SDL_Rect statusBar = {0, windowHeight - 80, windowWidth, 0};
    
    std::string status = "Ready";
    if (daw) {
        if (daw->isPlaying()) status = "Playing";
        else if (daw->isRecording()) status = "Recording";
    }
    
    drawText("Status: " + status, 800, windowHeight - 40, colors.text);
    
    // CPU meter
    std::string cpuStr = "CPU: 0%";
    if (daw && daw->getAudioEngine()) {
        float cpu = daw->getAudioEngine()->getCPULoad();
        cpuStr = "CPU: " + std::to_string((int)(cpu * 100)) + "%";
    }
    drawText(cpuStr, 950, windowHeight - 40, colors.text);
}

void DAWGUI::drawButton(const Button& button) {
    SDL_Color bgColor = colors.button;
    if (button.pressed) {
        bgColor = colors.buttonActive;
    } else if (button.hovered) {
        bgColor = colors.buttonHover;
    }
    
    drawRect(button.rect, bgColor);
    drawRect(button.rect, colors.border, false);
    
    // Center text
    int textX = button.rect.x + (button.rect.w - button.label.length() * 6) / 2;
    int textY = button.rect.y + (button.rect.h - 12) / 2;
    drawText(button.label, textX, textY, colors.text);
}

void DAWGUI::drawFader(const Fader& fader) {
    // Fader track
    drawRect(fader.rect, colors.border);
    
    // Fader thumb
    int thumbY = fader.rect.y + (int)((1.0f - fader.value) * fader.rect.h);
    SDL_Rect thumb = {fader.rect.x - 2, thumbY - 5, fader.rect.w + 4, 10};
    drawRect(thumb, fader.dragging ? colors.accent : colors.button);
}

void DAWGUI::drawMeter(const Meter& meter) {
    // Meter background
    drawRect(meter.rect, colors.border);
    
    // Level indicator
    int levelHeight = (int)(meter.level * meter.rect.h);
    SDL_Rect levelRect = {meter.rect.x, meter.rect.y + meter.rect.h - levelHeight,
                          meter.rect.w, levelHeight};
    drawRect(levelRect, colors.meter);
    
    // Peak indicator
    if (meter.peak > 0.01f) {
        int peakY = meter.rect.y + (int)((1.0f - meter.peak) * meter.rect.h);
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawLine(renderer, meter.rect.x, peakY, 
                          meter.rect.x + meter.rect.w, peakY);
    }
}

void DAWGUI::drawText(const std::string& text, int x, int y, SDL_Color color) {
    // Simple text rendering (in real app, use SDL_ttf)
    // For now, just draw text background to show where it would be
    SDL_Rect textRect = {x, y, (int)text.length() * 8, 14};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    // Just draw the rect outline for now
    SDL_RenderDrawRect(renderer, &textRect);
}

void DAWGUI::drawRect(const SDL_Rect& rect, SDL_Color color, bool filled) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    if (filled) {
        SDL_RenderFillRect(renderer, &rect);
    } else {
        SDL_RenderDrawRect(renderer, &rect);
    }
}

void DAWGUI::drawLine(int x1, int y1, int x2, int y2, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

void DAWGUI::handleMouseDown(int x, int y) {
    // Check transport buttons
    if (isPointInRect(x, y, playButton.rect)) {
        playButton.pressed = true;
        if (daw) daw->play();
    } else if (isPointInRect(x, y, stopButton.rect)) {
        stopButton.pressed = true;
        if (daw) daw->stop();
    } else if (isPointInRect(x, y, recordButton.rect)) {
        recordButton.pressed = true;
        if (daw) daw->record();
    } else if (isPointInRect(x, y, pauseButton.rect)) {
        pauseButton.pressed = true;
        if (daw) daw->pause();
    }
    
    // Check faders
    for (auto& fader : channelFaders) {
        if (isPointInRect(x, y, fader.rect)) {
            fader.dragging = true;
            fader.value = 1.0f - (float)(y - fader.rect.y) / fader.rect.h;
            fader.value = std::max(0.0f, std::min(1.0f, fader.value));
        }
    }
}

void DAWGUI::handleMouseUp(int x, int y) {
    playButton.pressed = false;
    stopButton.pressed = false;
    recordButton.pressed = false;
    pauseButton.pressed = false;
    
    for (auto& fader : channelFaders) {
        fader.dragging = false;
    }
}

void DAWGUI::handleMouseMove(int x, int y) {
    // Update hover states
    playButton.hovered = isPointInRect(x, y, playButton.rect);
    stopButton.hovered = isPointInRect(x, y, stopButton.rect);
    recordButton.hovered = isPointInRect(x, y, recordButton.rect);
    pauseButton.hovered = isPointInRect(x, y, pauseButton.rect);
    
    // Update dragging faders
    for (auto& fader : channelFaders) {
        if (fader.dragging) {
            fader.value = 1.0f - (float)(y - fader.rect.y) / fader.rect.h;
            fader.value = std::max(0.0f, std::min(1.0f, fader.value));
            
            // Apply to mixer if available
            // TODO: Connect to actual mixer channels
        }
    }
}

void DAWGUI::handleKeyDown(SDL_Keycode key) {
    switch (key) {
        case SDLK_SPACE:
            if (daw) {
                if (daw->isPlaying()) {
                    daw->stop();
                } else {
                    daw->play();
                }
            }
            break;
            
        case SDLK_ESCAPE:
            quit = true;
            break;
            
        case SDLK_r:
            if (daw) daw->record();
            break;
    }
}

bool DAWGUI::isPointInRect(int x, int y, const SDL_Rect& rect) {
    return x >= rect.x && x < rect.x + rect.w &&
           y >= rect.y && y < rect.y + rect.h;
}

} // namespace OmegaDAW
