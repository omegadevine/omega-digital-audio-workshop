#include "UIMixer.h"
#include <iostream>
#include <algorithm>

namespace OmegaDAW {

// UIChannelStrip Implementation
UIChannelStrip::UIChannelStrip(const std::string& id, const std::string& name)
    : UIComponent(id), channelName(name), 
      volumeLevel(0.75f), panPosition(0.0f), 
      muted(false), soloed(false), peakLevel(0.0f) {
    
    backgroundColor = Color(0.22f, 0.22f, 0.25f, 1.0f);
    
    // Create child controls
    nameLabel = std::make_shared<UILabel>(id + "_name", name);
    volumeLabel = std::make_shared<UILabel>(id + "_vol_label", "Vol");
    panLabel = std::make_shared<UILabel>(id + "_pan_label", "Pan");
    
    volumeSlider = std::make_shared<UISlider>(id + "_volume", 0.0f, 1.0f, false);
    volumeSlider->setValue(volumeLevel);
    volumeSlider->setOnValueChanged([this](float v) { this->volumeLevel = v; });
    
    panSlider = std::make_shared<UISlider>(id + "_pan", -1.0f, 1.0f, true);
    panSlider->setValue(panPosition);
    panSlider->setOnValueChanged([this](float v) { this->panPosition = v; });
    
    muteToggle = std::make_shared<UIToggle>(id + "_mute", "M");
    muteToggle->setOnToggled([this](bool m) { this->muted = m; });
    
    soloToggle = std::make_shared<UIToggle>(id + "_solo", "S");
    soloToggle->setOnToggled([this](bool s) { this->soloed = s; });
}

void UIChannelStrip::render() {
    // Render channel strip background
    // Render name label
    // Render volume slider
    // Render pan slider
    // Render mute and solo buttons
    // Render peak meter
    
    if (nameLabel) nameLabel->render();
    if (volumeLabel) volumeLabel->render();
    if (panLabel) panLabel->render();
    if (volumeSlider) volumeSlider->render();
    if (panSlider) panSlider->render();
    if (muteToggle) muteToggle->render();
    if (soloToggle) soloToggle->render();
    
    // Draw peak meter visualization
    float meterHeight = bounds.height * 0.4f;
    float meterY = bounds.y + 100;
    // Draw meter bar based on peakLevel
}

void UIChannelStrip::update(float deltaTime) {
    // Decay peak level
    peakLevel *= 0.95f;
}

void UIChannelStrip::handleMouseDown(float x, float y) {
    if (volumeSlider) volumeSlider->handleMouseDown(x, y);
    if (panSlider) panSlider->handleMouseDown(x, y);
    if (muteToggle) muteToggle->handleMouseDown(x, y);
    if (soloToggle) soloToggle->handleMouseDown(x, y);
}

void UIChannelStrip::handleMouseUp(float x, float y) {
    if (volumeSlider) volumeSlider->handleMouseUp(x, y);
    if (panSlider) panSlider->handleMouseUp(x, y);
}

void UIChannelStrip::handleMouseMove(float x, float y) {
    if (volumeSlider) volumeSlider->handleMouseMove(x, y);
    if (panSlider) panSlider->handleMouseMove(x, y);
}

void UIChannelStrip::setVolume(float volume) {
    volumeLevel = std::max(0.0f, std::min(1.0f, volume));
    if (volumeSlider) volumeSlider->setValue(volumeLevel);
}

void UIChannelStrip::setPan(float pan) {
    panPosition = std::max(-1.0f, std::min(1.0f, pan));
    if (panSlider) panSlider->setValue(panPosition);
}

void UIChannelStrip::setMute(bool mute) {
    muted = mute;
    if (muteToggle) muteToggle->setChecked(muted);
}

void UIChannelStrip::setSolo(bool solo) {
    soloed = solo;
    if (soloToggle) soloToggle->setChecked(soloed);
}

// UIMixer Implementation
UIMixer::UIMixer(const std::string& id)
    : UIComponent(id), channelWidth(80.0f), scrollOffset(0.0f) {
    
    backgroundColor = Color(0.18f, 0.18f, 0.2f, 1.0f);
    
    // Create master channel
    masterChannel = std::make_shared<UIChannelStrip>(id + "_master", "Master");
}

void UIMixer::render() {
    // Render mixer background
    
    // Render channel strips
    float x = bounds.x + scrollOffset;
    for (auto& strip : channelStrips) {
        if (strip && x < bounds.x + bounds.width) {
            Rect stripBounds(x, bounds.y, channelWidth, bounds.height - 100);
            strip->setBounds(stripBounds);
            strip->render();
        }
        x += channelWidth + 10;
    }
    
    // Render master channel on the right
    if (masterChannel) {
        float masterX = bounds.x + bounds.width - channelWidth - 10;
        Rect masterBounds(masterX, bounds.y, channelWidth, bounds.height - 100);
        masterChannel->setBounds(masterBounds);
        masterChannel->render();
    }
}

void UIMixer::update(float deltaTime) {
    for (auto& strip : channelStrips) {
        if (strip) strip->update(deltaTime);
    }
    if (masterChannel) masterChannel->update(deltaTime);
}

void UIMixer::handleMouseDown(float x, float y) {
    if (!bounds.contains(x, y)) return;
    
    for (auto& strip : channelStrips) {
        if (strip) strip->handleMouseDown(x, y);
    }
    if (masterChannel) masterChannel->handleMouseDown(x, y);
}

void UIMixer::handleMouseUp(float x, float y) {
    for (auto& strip : channelStrips) {
        if (strip) strip->handleMouseUp(x, y);
    }
    if (masterChannel) masterChannel->handleMouseUp(x, y);
}

void UIMixer::handleMouseMove(float x, float y) {
    for (auto& strip : channelStrips) {
        if (strip) strip->handleMouseMove(x, y);
    }
    if (masterChannel) masterChannel->handleMouseMove(x, y);
}

void UIMixer::addChannel(const std::string& name) {
    std::string id = std::string(getId()) + "_ch" + std::to_string(channelStrips.size());
    auto strip = std::make_shared<UIChannelStrip>(id, name);
    channelStrips.push_back(strip);
}

void UIMixer::removeChannel(const std::string& name) {
    channelStrips.erase(
        std::remove_if(channelStrips.begin(), channelStrips.end(),
            [&name](const std::shared_ptr<UIChannelStrip>& strip) {
                return strip->getId().find(name) != std::string::npos;
            }),
        channelStrips.end()
    );
}

std::shared_ptr<UIChannelStrip> UIMixer::getChannel(const std::string& name) {
    auto it = std::find_if(channelStrips.begin(), channelStrips.end(),
        [&name](const std::shared_ptr<UIChannelStrip>& strip) {
            return strip->getId().find(name) != std::string::npos;
        });
    
    return (it != channelStrips.end()) ? *it : nullptr;
}

void UIMixer::clearChannels() {
    channelStrips.clear();
}

void UIMixer::scrollHorizontal(float delta) {
    scrollOffset += delta;
    
    // Clamp scroll offset
    float maxOffset = 0.0f;
    float minOffset = -(channelStrips.size() * (channelWidth + 10) - bounds.width + 100);
    scrollOffset = std::max(minOffset, std::min(maxOffset, scrollOffset));
}

}
