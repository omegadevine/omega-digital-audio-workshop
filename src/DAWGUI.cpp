#include "DAWGUI.h"
#include "Clip.h"
#include <iostream>
#include <cstdlib>

namespace OmegaDAW {

DAWGUI::DAWGUI(DAWApplication* app)
    : daw(app), window(nullptr), renderer(nullptr), font(nullptr),
      quit(false), windowWidth(0), windowHeight(0),
      mouseX(0), mouseY(0),
      showingFileDialog(false), fileDialogMode(""), fileDialogPath("") {
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
    
    // Initialize SDL_ttf
    if (TTF_Init() < 0) {
        std::cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
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
    
    // Load font (try multiple locations)
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
        std::cerr << "Failed to load any font: " << TTF_GetError() << std::endl;
        std::cerr << "Text will not be rendered properly." << std::endl;
    }
    
    // Setup transport buttons
    playButton.label = "Play";
    playButton.rect = {20, windowHeight - 60, 80, 40};
    playButton.hovered = false;
    playButton.pressed = false;
    
    stopButton.label = "Stop";
    stopButton.rect = {110, windowHeight - 60, 80, 40};
    stopButton.hovered = false;
    stopButton.pressed = false;
    
    recordButton.label = "Rec";
    recordButton.rect = {200, windowHeight - 60, 80, 40};
    recordButton.hovered = false;
    recordButton.pressed = false;
    
    pauseButton.label = "Pause";
    pauseButton.rect = {290, windowHeight - 60, 80, 40};
    pauseButton.hovered = false;
    pauseButton.pressed = false;
    
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
    
    // Setup menus
    setupMenus();
    
    std::cout << "GUI initialized successfully" << std::endl;
    return true;
}

void DAWGUI::shutdown() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    TTF_Quit();
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
    
    // Render panels (bottom layer)
    renderTimelinePanel();
    renderMixerPanel();
    renderTransportPanel();
    renderStatusBar();
    
    // Render menu bar and dropdowns (top layer)
    renderMenuBar();
    
    // Render file dialog on top if showing
    if (showingFileDialog) {
        renderFileDialog();
    }
    
    // Present
    SDL_RenderPresent(renderer);
}

void DAWGUI::renderMenuBar() {
    SDL_Rect menuBar = {0, 0, windowWidth, 30};
    drawRect(menuBar, colors.panel);
    
    // Draw menus
    for (auto& menu : menus) {
        // Draw menu label
        SDL_Color labelColor = menu.hovered ? colors.accent : colors.text;
        drawText(menu.label, menu.rect.x + 5, menu.rect.y + 8, labelColor);
        
        // Draw dropdown if open
        if (menu.open) {
            // Dropdown background
            int dropdownHeight = menu.items.size() * 25 + 10;
            SDL_Rect dropdownRect = {menu.rect.x, menu.rect.y + menu.rect.h, 150, dropdownHeight};
            drawRect(dropdownRect, colors.panel);
            drawRect(dropdownRect, colors.border, false);
            
            // Draw menu items
            for (size_t i = 0; i < menu.items.size(); ++i) {
                auto& item = menu.items[i];
                SDL_Color itemColor = item.enabled ? 
                    (item.hovered ? colors.accent : colors.text) : 
                    SDL_Color{100, 100, 100, 255};
                
                if (item.hovered && item.enabled) {
                    SDL_Rect highlightRect = {dropdownRect.x + 2, dropdownRect.y + 5 + (int)i * 25, 146, 23};
                    drawRect(highlightRect, colors.buttonHover);
                }
                
                drawText(item.label, dropdownRect.x + 10, dropdownRect.y + 10 + i * 25, itemColor);
            }
        }
    }
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
    
    // Draw track channel strips
    if (daw && daw->getProject()) {
        int numTracks = std::min(4, daw->getProject()->getNumTracks()); // Show max 4 tracks
        
        for (int i = 0; i < numTracks; ++i) {
            auto track = daw->getProject()->getTrack(i);
            if (!track) continue;
            
            int channelX = windowWidth - 200 + i * 50;
            int channelY = 70;
            
            // Update fader from track volume
            if (i < static_cast<int>(channelFaders.size())) {
                channelFaders[i].value = track->getVolume();
                channelFaders[i].label = track->getName();
                channelFaders[i].rect.x = channelX;
                channelFaders[i].rect.y = channelY;
            }
            
            // Update meter levels from playback
            if (i < static_cast<int>(channelMeters.size())) {
                if (daw && daw->isPlaying() && !track->isMuted()) {
                    channelMeters[i].level = 0.3f + (rand() % 40) / 100.0f;
                    channelMeters[i].peak = std::max(channelMeters[i].peak, channelMeters[i].level);
                } else {
                    channelMeters[i].level *= 0.95f;
                    channelMeters[i].peak *= 0.98f;
                }
                
                channelMeters[i].rect.x = channelX + 5;
                channelMeters[i].rect.y = channelY - 120;
                
                drawMeter(channelMeters[i]);
            }
            
            // Draw fader
            if (i < static_cast<int>(channelFaders.size())) {
                drawFader(channelFaders[i]);
                
                // Draw track name below fader
                std::string trackName = track->getName();
                if (trackName.length() > 6) trackName = trackName.substr(0, 6);
                drawText(trackName, channelX - 5, channelY + 105, colors.text);
                
                // Draw mute/solo indicators
                if (track->isMuted()) {
                    SDL_Rect muteRect = {channelX + 2, channelY + 120, 15, 12};
                    drawRect(muteRect, {255, 100, 100, 255});
                    drawText("M", channelX + 4, channelY + 121, {0, 0, 0, 255});
                }
                if (track->isSoloed()) {
                    SDL_Rect soloRect = {channelX + 20, channelY + 120, 15, 12};
                    drawRect(soloRect, {255, 255, 100, 255});
                    drawText("S", channelX + 22, channelY + 121, {0, 0, 0, 255});
                }
            }
        }
    } else {
        // Fallback to original behavior for channel faders
        for (size_t i = 0; i < channelFaders.size(); ++i) {
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
}

void DAWGUI::renderTimelinePanel() {
    // Timeline panel background
    SDL_Rect timelinePanel = {0, 30, windowWidth - 220, windowHeight - 110};
    drawRect(timelinePanel, colors.background);
    
    drawText("TIMELINE / ARRANGEMENT", 20, 40, colors.accent);
    
    // Calculate visible time range
    const double pixelsPerSecond = 50.0; // 50 pixels = 1 second
    const int timelineWidth = windowWidth - 220;
    const int timelineStartX = 20;
    const int timelineStartY = 80;
    const int trackHeight = 60;
    const int trackSpacing = 10;
    
    // Draw timeline ruler
    SDL_SetRenderDrawColor(renderer, colors.border.r, colors.border.g, 
                          colors.border.b, colors.border.a);
    SDL_RenderDrawLine(renderer, 0, timelineStartY - 5, timelineWidth, timelineStartY - 5);
    
    // Draw time markers
    for (int sec = 0; sec < 30; ++sec) {
        int x = timelineStartX + (int)(sec * pixelsPerSecond);
        if (x > timelineWidth - 50) break;
        
        SDL_RenderDrawLine(renderer, x, timelineStartY - 10, x, timelineStartY - 5);
        drawText(std::to_string(sec) + "s", x - 10, timelineStartY - 25, colors.text);
    }
    
    // Draw playhead
    if (daw && daw->getTransport()) {
        double pos = daw->getTransport()->getPosition();
        int playheadX = timelineStartX + (int)(pos * pixelsPerSecond);
        SDL_SetRenderDrawColor(renderer, colors.accent.r, colors.accent.g, 
                              colors.accent.b, colors.accent.a);
        SDL_RenderDrawLine(renderer, playheadX, timelineStartY - 10, playheadX, windowHeight - 110);
    }
    
    // Draw tracks and clips
    if (daw && daw->getProject()) {
        int numTracks = daw->getProject()->getNumTracks();
        
        for (int trackIdx = 0; trackIdx < numTracks; ++trackIdx) {
            auto track = daw->getProject()->getTrack(trackIdx);
            if (!track) continue;
            
            int trackY = timelineStartY + trackIdx * (trackHeight + trackSpacing);
            
            // Draw track background
            SDL_Rect trackRect = {0, trackY, timelineWidth, trackHeight};
            SDL_Color trackBg = {35, 35, 40, 255};
            drawRect(trackRect, trackBg);
            drawRect(trackRect, colors.border, false);
            
            // Draw track label
            drawText(track->getName(), 5, trackY + 5, colors.accent);
            
            // Draw clips on this track
            const auto& clips = track->getClips();
            for (const auto& clip : clips) {
                if (!clip) continue;
                
                // Calculate clip position and size
                int clipX = timelineStartX + (int)(clip->getStartTime() * pixelsPerSecond);
                int clipWidth = (int)(clip->getDuration() * pixelsPerSecond);
                int clipY = trackY + 20;
                int clipHeight = trackHeight - 25;
                
                SDL_Rect clipRect = {clipX, clipY, clipWidth, clipHeight};
                
                // Choose color based on clip type
                SDL_Color clipColor;
                unsigned int color = clip->getColor();
                clipColor.r = (color >> 16) & 0xFF;
                clipColor.g = (color >> 8) & 0xFF;
                clipColor.b = color & 0xFF;
                clipColor.a = 255;
                
                // Draw clip background
                drawRect(clipRect, clipColor);
                
                // Draw clip border
                SDL_Color borderColor = {
                    static_cast<Uint8>(clipColor.r * 0.6),
                    static_cast<Uint8>(clipColor.g * 0.6),
                    static_cast<Uint8>(clipColor.b * 0.6),
                    255
                };
                drawRect(clipRect, borderColor, false);
                
                // Draw clip name (if it fits)
                if (clipWidth > 40) {
                    drawText(clip->getName(), clipX + 5, clipY + 5, colors.text);
                }
                
                // Draw waveform or MIDI notes indicator
                if (clip->getType() == ClipType::Audio) {
                    // Draw simple waveform placeholder
                    SDL_SetRenderDrawColor(renderer, 100, 255, 100, 100);
                    for (int i = 0; i < clipWidth - 4; i += 4) {
                        int waveHeight = (rand() % (clipHeight - 10)) / 2;
                        int midY = clipY + clipHeight / 2;
                        SDL_RenderDrawLine(renderer, clipX + 2 + i, midY - waveHeight, 
                                         clipX + 2 + i, midY + waveHeight);
                    }
                } else if (clip->getType() == ClipType::MIDI) {
                    // Draw MIDI note indicators
                    auto midiClip = std::static_pointer_cast<MIDIClip>(clip);
                    const auto& notes = midiClip->getNotes();
                    
                    for (const auto& note : notes) {
                        if (note.isNoteOn()) {
                            double noteTime = note.getTimestamp();
                            int noteX = clipX + (int)((noteTime - clip->getOffset()) * pixelsPerSecond);
                            int noteY = clipY + 2 + ((127 - note.getNoteNumber()) * (clipHeight - 4)) / 128;
                            
                            SDL_Rect noteRect = {noteX, noteY, 3, 3};
                            SDL_SetRenderDrawColor(renderer, 255, 255, 100, 255);
                            SDL_RenderFillRect(renderer, &noteRect);
                        }
                    }
                }
            }
        }
    } else {
        // Show message when no project
        drawText("No project loaded. Create a new project to begin.", 
                 timelineStartX, timelineStartY + 50, colors.text);
    }
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
    std::cout << "Mouse clicked at: (" << x << ", " << y << ")" << std::endl;
    
    // Check menu bar clicks
    if (y < 30) {
        std::cout << "Click in menu bar area." << std::endl;
        for (size_t i = 0; i < menus.size(); ++i) {
            std::cout << "  Checking menu " << i << " (" << menus[i].label << "): rect=("  
                      << menus[i].rect.x << "," << menus[i].rect.y << ","
                      << menus[i].rect.w << "," << menus[i].rect.h << ") open=" << menus[i].open << std::endl;
            if (isPointInRect(x, y, menus[i].rect)) {
                // Toggle menu
                bool wasOpen = menus[i].open;
                closeAllMenus();
                menus[i].open = !wasOpen;
                std::cout << "Menu '" << menus[i].label << "' clicked. WasOpen: " << wasOpen << ", Now open: " << menus[i].open << std::endl;
                return;
            }
            
            // Check if clicking on dropdown
            if (menus[i].open) {
                int dropdownHeight = menus[i].items.size() * 25 + 10;
                SDL_Rect dropdownRect = {menus[i].rect.x, menus[i].rect.y + menus[i].rect.h, 150, dropdownHeight};
                if (isPointInRect(x, y, dropdownRect)) {
                    int itemIndex = (y - dropdownRect.y - 5) / 25;
                    if (itemIndex >= 0 && itemIndex < (int)menus[i].items.size()) {
                        if (menus[i].items[itemIndex].enabled) {
                            handleMenuClick(i, itemIndex);
                            closeAllMenus();
                        }
                    }
                    return;
                }
            }
        }
        closeAllMenus();
        return;
    }
    
    // Close menus if clicking elsewhere
    closeAllMenus();
    
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
    // Update menu hover states
    for (auto& menu : menus) {
        menu.hovered = isPointInRect(x, y, menu.rect);
        
        if (menu.open) {
            int dropdownHeight = menu.items.size() * 25 + 10;
            SDL_Rect dropdownRect = {menu.rect.x, menu.rect.y + menu.rect.h, 150, dropdownHeight};
            
            for (auto& item : menu.items) {
                item.hovered = false;
            }
            
            if (isPointInRect(x, y, dropdownRect)) {
                int itemIndex = (y - dropdownRect.y - 5) / 25;
                if (itemIndex >= 0 && itemIndex < (int)menu.items.size()) {
                    menu.items[itemIndex].hovered = true;
                }
            }
        }
    }
    
    // Update hover states
    playButton.hovered = isPointInRect(x, y, playButton.rect);
    stopButton.hovered = isPointInRect(x, y, stopButton.rect);
    recordButton.hovered = isPointInRect(x, y, recordButton.rect);
    pauseButton.hovered = isPointInRect(x, y, pauseButton.rect);
    
    // Update dragging faders
    for (size_t i = 0; i < channelFaders.size(); ++i) {
        auto& fader = channelFaders[i];
        if (fader.dragging) {
            fader.value = 1.0f - (float)(y - fader.rect.y) / fader.rect.h;
            fader.value = std::max(0.0f, std::min(1.0f, fader.value));
            
            // Apply to mixer if available
            if (daw && daw->getMixer()) {
                auto mixer = daw->getMixer();
                if (i < static_cast<size_t>(mixer->getNumChannels())) {
                    auto channel = mixer->getChannel(static_cast<int>(i));
                    if (channel) {
                        // Convert 0-1 to -60 to +6 dB range
                        float db = (fader.value * 66.0f) - 60.0f;
                        channel->setVolume(db);
                    }
                }
            }
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

void DAWGUI::setupMenus() {
    // File menu
    Menu fileMenu;
    fileMenu.label = "File";
    fileMenu.rect = {5, 0, 50, 30};  // Made wider and starts at x=5
    fileMenu.open = false;
    
    MenuItem newProject;
    newProject.label = "New Project";
    newProject.enabled = true;
    fileMenu.items.push_back(newProject);
    
    MenuItem openProject;
    openProject.label = "Open Project...";
    openProject.enabled = true;
    fileMenu.items.push_back(openProject);
    
    MenuItem saveProject;
    saveProject.label = "Save";
    saveProject.enabled = true;
    fileMenu.items.push_back(saveProject);
    
    MenuItem saveAsProject;
    saveAsProject.label = "Save As...";
    saveAsProject.enabled = true;
    fileMenu.items.push_back(saveAsProject);
    
    MenuItem separator;
    separator.label = "---";
    separator.enabled = false;
    fileMenu.items.push_back(separator);
    
    MenuItem exitItem;
    exitItem.label = "Exit";
    exitItem.enabled = true;
    fileMenu.items.push_back(exitItem);
    
    menus.push_back(fileMenu);
    
    // Edit menu (placeholder)
    Menu editMenu;
    editMenu.label = "Edit";
    editMenu.rect = {60, 0, 50, 30};  // Made wider
    editMenu.open = false;
    menus.push_back(editMenu);
    
    // View menu (placeholder)
    Menu viewMenu;
    viewMenu.label = "View";
    viewMenu.rect = {115, 0, 50, 30};  // Made wider
    viewMenu.open = false;
    menus.push_back(viewMenu);
    
    std::cout << "Menus initialized. Total menus: " << menus.size() << std::endl;
    std::cout << "File menu items: " << fileMenu.items.size() << std::endl;
    
    // Verify initial state
    for (size_t i = 0; i < menus.size(); ++i) {
        std::cout << "  Menu " << i << " (" << menus[i].label << "): open=" << menus[i].open << std::endl;
    }
}

void DAWGUI::handleMenuClick(int menuIndex, int itemIndex) {
    if (menuIndex < 0 || menuIndex >= (int)menus.size()) return;
    if (itemIndex < 0 || itemIndex >= (int)menus[menuIndex].items.size()) return;
    
    const std::string& menuLabel = menus[menuIndex].label;
    const std::string& itemLabel = menus[menuIndex].items[itemIndex].label;
    
    if (menuLabel == "File") {
        if (itemLabel == "New Project") {
            if (daw) {
                std::cout << "Creating new project..." << std::endl;
                daw->newProject("Untitled Project");
            }
        } else if (itemLabel == "Open Project...") {
            showFileDialog("open");
        } else if (itemLabel == "Save") {
            if (daw && daw->getProject()) {
                std::string filepath = daw->getProject()->getFilePath();
                if (filepath.empty()) {
                    showFileDialog("saveas");
                } else {
                    std::cout << "Saving project to: " << filepath << std::endl;
                    daw->saveProject(filepath);
                }
            }
        } else if (itemLabel == "Save As...") {
            showFileDialog("saveas");
        } else if (itemLabel == "Exit") {
            quit = true;
        }
    }
}

void DAWGUI::closeAllMenus() {
    for (auto& menu : menus) {
        menu.open = false;
    }
}

void DAWGUI::showFileDialog(const std::string& mode) {
    fileDialogMode = mode;
    fileDialogPath = getWindowsFilePath(mode);
    
    if (!fileDialogPath.empty()) {
        executeFileOperation();
    }
}

void DAWGUI::executeFileOperation() {
    if (!daw || fileDialogPath.empty()) return;
    
    if (fileDialogMode == "open") {
        std::cout << "Loading project from: " << fileDialogPath << std::endl;
        if (daw->loadProject(fileDialogPath)) {
            std::cout << "Project loaded successfully!" << std::endl;
        } else {
            std::cerr << "Failed to load project" << std::endl;
        }
    } else if (fileDialogMode == "save" || fileDialogMode == "saveas") {
        std::cout << "Saving project to: " << fileDialogPath << std::endl;
        if (daw->saveProject(fileDialogPath)) {
            std::cout << "Project saved successfully!" << std::endl;
        } else {
            std::cerr << "Failed to save project" << std::endl;
        }
    }
    
    fileDialogPath = "";
    fileDialogMode = "";
}

std::string DAWGUI::getWindowsFilePath(const std::string& mode) {
    // Use Windows command-line for file selection (simple implementation)
    // In a full implementation, you would use Windows File Dialog API
    
    std::string filter = ".dawproj";
    std::string defaultPath = "project.dawproj";
    
    if (mode == "open") {
        std::cout << "\n=== Open Project ===" << std::endl;
        std::cout << "Enter project file path (or press Enter to cancel): ";
    } else if (mode == "save" || mode == "saveas") {
        std::cout << "\n=== Save Project ===" << std::endl;
        std::cout << "Enter project file path (or press Enter for default '" << defaultPath << "'): ";
    }
    
    // For now, return a default path - in a real implementation,
    // you'd use Windows API to show a proper file dialog
    // This is a simplified version for demonstration
    
    if (mode == "save" || mode == "saveas") {
        if (daw && daw->getProject()) {
            std::string existingPath = daw->getProject()->getFilePath();
            if (!existingPath.empty() && mode == "save") {
                return existingPath;
            }
            
            // Generate default filename based on project name
            std::string projectName = daw->getProject()->getName();
            std::string filename = projectName + ".dawproj";
            
            // Remove invalid characters
            for (char& c : filename) {
                if (c == ' ') c = '_';
                if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
                    c = '_';
                }
            }
            
            return filename;
        }
        return defaultPath;
    }
    
    return ""; // Cancel for open dialog (would need actual implementation)
}

void DAWGUI::renderFileDialog() {
    // Render a semi-transparent overlay
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect overlay = {0, 0, windowWidth, windowHeight};
    SDL_RenderFillRect(renderer, &overlay);
    
    // Render dialog box
    SDL_Rect dialogBox = {windowWidth/2 - 200, windowHeight/2 - 100, 400, 200};
    drawRect(dialogBox, colors.panel);
    drawRect(dialogBox, colors.border, false);
    
    // Render dialog content
    std::string title = "File Dialog";
    if (fileDialogMode == "open") title = "Open Project";
    else if (fileDialogMode == "save" || fileDialogMode == "saveas") title = "Save Project";
    
    drawText(title, dialogBox.x + 20, dialogBox.y + 20, colors.accent);
    drawText("Path: " + fileDialogPath, dialogBox.x + 20, dialogBox.y + 60, colors.text);
    drawText("(Check console for input)", dialogBox.x + 20, dialogBox.y + 100, colors.text);
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

} // namespace OmegaDAW
