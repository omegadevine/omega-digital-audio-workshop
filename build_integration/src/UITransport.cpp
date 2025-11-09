#include "UITransport.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace OmegaDAW {

UITransport::UITransport(const std::string& id, Transport* transport)
    : UIComponent(id), transport(transport), 
      isPlaying(false), isRecording(false), isLooping(false),
      currentTime(0.0), currentTempo(120.0f) {
    
    backgroundColor = Color(0.25f, 0.25f, 0.28f, 1.0f);
    
    // Create transport buttons
    playButton = std::make_shared<UIButton>(id + "_play", "Play");
    playButton->setOnClick([this]() { this->onPlay(); });
    
    pauseButton = std::make_shared<UIButton>(id + "_pause", "Pause");
    pauseButton->setOnClick([this]() { this->onPause(); });
    
    stopButton = std::make_shared<UIButton>(id + "_stop", "Stop");
    stopButton->setOnClick([this]() { this->onStop(); });
    
    recordButton = std::make_shared<UIButton>(id + "_record", "Record");
    recordButton->setOnClick([this]() { this->onRecord(); });
    
    loopButton = std::make_shared<UIButton>(id + "_loop", "Loop");
    loopButton->setOnClick([this]() { this->onLoop(); });
    
    // Create display labels
    timeLabel = std::make_shared<UILabel>(id + "_time", "00:00:00.000");
    timeLabel->setFontSize(18.0f);
    
    tempoLabel = std::make_shared<UILabel>(id + "_tempo_label", "BPM:");
    
    // Create tempo input
    tempoInput = std::make_shared<UITextBox>(id + "_tempo_input");
    tempoInput->setText("120.0");
}

void UITransport::render() {
    // Render transport bar background
    
    // Position and render buttons
    float buttonWidth = 60.0f;
    float buttonHeight = 30.0f;
    float buttonSpacing = 10.0f;
    float x = bounds.x + 20;
    float y = bounds.y + (bounds.height - buttonHeight) / 2;
    
    if (playButton) {
        playButton->setBounds(Rect(x, y, buttonWidth, buttonHeight));
        playButton->render();
        x += buttonWidth + buttonSpacing;
    }
    
    if (pauseButton) {
        pauseButton->setBounds(Rect(x, y, buttonWidth, buttonHeight));
        pauseButton->render();
        x += buttonWidth + buttonSpacing;
    }
    
    if (stopButton) {
        stopButton->setBounds(Rect(x, y, buttonWidth, buttonHeight));
        stopButton->render();
        x += buttonWidth + buttonSpacing;
    }
    
    if (recordButton) {
        recordButton->setBounds(Rect(x, y, buttonWidth, buttonHeight));
        recordButton->render();
        x += buttonWidth + buttonSpacing;
    }
    
    if (loopButton) {
        loopButton->setBounds(Rect(x, y, buttonWidth, buttonHeight));
        loopButton->render();
        x += buttonWidth + buttonSpacing;
    }
    
    // Render time display
    if (timeLabel) {
        float timeX = bounds.x + bounds.width - 300;
        timeLabel->setBounds(Rect(timeX, y, 150, buttonHeight));
        timeLabel->render();
    }
    
    // Render tempo controls
    if (tempoLabel && tempoInput) {
        float tempoX = bounds.x + bounds.width - 130;
        tempoLabel->setBounds(Rect(tempoX, y, 50, buttonHeight));
        tempoLabel->render();
        
        tempoInput->setBounds(Rect(tempoX + 55, y, 60, buttonHeight));
        tempoInput->render();
    }
}

void UITransport::update(float deltaTime) {
    if (transport) {
        isPlaying = transport->isPlaying();
        currentTime = transport->getPositionSeconds();
        currentTempo = transport->getTempo();
    }
    
    updateTimeDisplay();
}

void UITransport::handleMouseDown(float x, float y) {
    if (!bounds.contains(x, y)) return;
    
    if (playButton) playButton->handleMouseDown(x, y);
    if (pauseButton) pauseButton->handleMouseDown(x, y);
    if (stopButton) stopButton->handleMouseDown(x, y);
    if (recordButton) recordButton->handleMouseDown(x, y);
    if (loopButton) loopButton->handleMouseDown(x, y);
    if (tempoInput) tempoInput->handleMouseDown(x, y);
}

void UITransport::handleMouseUp(float x, float y) {
    if (playButton) playButton->handleMouseUp(x, y);
    if (pauseButton) pauseButton->handleMouseUp(x, y);
    if (stopButton) stopButton->handleMouseUp(x, y);
    if (recordButton) recordButton->handleMouseUp(x, y);
    if (loopButton) loopButton->handleMouseUp(x, y);
}

void UITransport::handleKeyDown(int key) {
    // Handle keyboard shortcuts
    if (key == 32) { // Space bar
        if (isPlaying) {
            onPause();
        } else {
            onPlay();
        }
    }
    
    if (tempoInput) tempoInput->handleKeyDown(key);
}

void UITransport::updateTimeDisplay() {
    if (timeLabel) {
        timeLabel->setText(formatTime(currentTime));
    }
    
    if (tempoInput) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << currentTempo;
        tempoInput->setText(oss.str());
    }
}

void UITransport::onPlay() {
    if (transport) {
        transport->play();
        isPlaying = true;
    }
    std::cout << "Transport: Play" << std::endl;
}

void UITransport::onPause() {
    if (transport) {
        transport->pause();
        isPlaying = false;
    }
    std::cout << "Transport: Pause" << std::endl;
}

void UITransport::onStop() {
    if (transport) {
        transport->stop();
        isPlaying = false;
    }
    std::cout << "Transport: Stop" << std::endl;
}

void UITransport::onRecord() {
    isRecording = !isRecording;
    std::cout << "Transport: Record " << (isRecording ? "ON" : "OFF") << std::endl;
}

void UITransport::onLoop() {
    isLooping = !isLooping;
    if (transport) {
        transport->setLooping(isLooping);
    }
    std::cout << "Transport: Loop " << (isLooping ? "ON" : "OFF") << std::endl;
}

void UITransport::onTempoChanged() {
    if (transport && tempoInput) {
        try {
            float newTempo = std::stof(tempoInput->getText());
            if (newTempo > 0 && newTempo <= 300) {
                transport->setTempo(newTempo);
                currentTempo = newTempo;
            }
        } catch (...) {
            // Invalid tempo input, ignore
        }
    }
}

std::string UITransport::formatTime(double seconds) {
    int hours = static_cast<int>(seconds / 3600);
    int minutes = static_cast<int>((seconds - hours * 3600) / 60);
    int secs = static_cast<int>(seconds - hours * 3600 - minutes * 60);
    int millis = static_cast<int>((seconds - static_cast<int>(seconds)) * 1000);
    
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << hours << ":"
        << std::setw(2) << minutes << ":"
        << std::setw(2) << secs << "."
        << std::setw(3) << millis;
    
    return oss.str();
}

}
