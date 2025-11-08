#ifndef UITRANSPORT_H
#define UITRANSPORT_H

#include "UIWindow.h"
#include "UIControls.h"
#include "Transport.h"
#include <memory>

namespace OmegaDAW {

class UITransport : public UIComponent {
private:
    std::shared_ptr<UIButton> playButton;
    std::shared_ptr<UIButton> pauseButton;
    std::shared_ptr<UIButton> stopButton;
    std::shared_ptr<UIButton> recordButton;
    std::shared_ptr<UIButton> loopButton;
    
    std::shared_ptr<UILabel> timeLabel;
    std::shared_ptr<UILabel> tempoLabel;
    
    std::shared_ptr<UITextBox> tempoInput;
    
    Transport* transport;
    bool isPlaying;
    bool isRecording;
    bool isLooping;
    
    double currentTime;
    float currentTempo;
    
public:
    UITransport(const std::string& id, Transport* transport);
    
    void render() override;
    void update(float deltaTime) override;
    void handleMouseDown(float x, float y) override;
    void handleMouseUp(float x, float y) override;
    void handleKeyDown(int key) override;
    
    void setTransport(Transport* t) { transport = t; }
    void updateTimeDisplay();
    
private:
    void onPlay();
    void onPause();
    void onStop();
    void onRecord();
    void onLoop();
    void onTempoChanged();
    
    std::string formatTime(double seconds);
};

}

#endif
