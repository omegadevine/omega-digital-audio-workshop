#ifndef UIMIXER_H
#define UIMIXER_H

#include "UIWindow.h"
#include "UIControls.h"
#include "MixerChannel.h"
#include <vector>
#include <memory>

namespace OmegaDAW {

class UIChannelStrip : public UIComponent {
private:
    std::string channelName;
    std::shared_ptr<UISlider> volumeSlider;
    std::shared_ptr<UISlider> panSlider;
    std::shared_ptr<UIToggle> muteToggle;
    std::shared_ptr<UIToggle> soloToggle;
    std::shared_ptr<UILabel> nameLabel;
    std::shared_ptr<UILabel> volumeLabel;
    std::shared_ptr<UILabel> panLabel;
    
    float volumeLevel;
    float panPosition;
    bool muted;
    bool soloed;
    
    float peakLevel;
    
public:
    UIChannelStrip(const std::string& id, const std::string& name);
    
    void render() override;
    void update(float deltaTime) override;
    void handleMouseDown(float x, float y) override;
    void handleMouseUp(float x, float y) override;
    void handleMouseMove(float x, float y) override;
    
    void setVolume(float volume);
    void setPan(float pan);
    void setMute(bool mute);
    void setSolo(bool solo);
    void setPeakLevel(float peak) { peakLevel = peak; }
    
    float getVolume() const { return volumeLevel; }
    float getPan() const { return panPosition; }
    bool isMuted() const { return muted; }
    bool isSoloed() const { return soloed; }
};

class UIMixer : public UIComponent {
private:
    std::vector<std::shared_ptr<UIChannelStrip>> channelStrips;
    float channelWidth;
    float scrollOffset;
    
    std::shared_ptr<UIChannelStrip> masterChannel;
    
public:
    UIMixer(const std::string& id);
    
    void render() override;
    void update(float deltaTime) override;
    void handleMouseDown(float x, float y) override;
    void handleMouseUp(float x, float y) override;
    void handleMouseMove(float x, float y) override;
    
    void addChannel(const std::string& name);
    void removeChannel(const std::string& name);
    std::shared_ptr<UIChannelStrip> getChannel(const std::string& name);
    void clearChannels();
    
    void scrollHorizontal(float delta);
    
    std::shared_ptr<UIChannelStrip> getMasterChannel() { return masterChannel; }
};

}

#endif
