#ifndef OMEGA_DAW_DELAY_H
#define OMEGA_DAW_DELAY_H

#include "AudioEngine.h"
#include <vector>

namespace OmegaDAW {

class Delay : public IAudioProcessor {
public:
    Delay(float delayTimeMs = 500.0f, float feedback = 0.5f, float mix = 0.5f);
    
    void setDelayTime(float delayTimeMs);
    void setFeedback(float feedback);
    void setMix(float mix);
    
    float getDelayTime() const { return delayTimeMs_; }
    float getFeedback() const { return feedback_; }
    float getMix() const { return mix_; }
    
    void prepare(int sampleRate, int maxBufferSize) override;
    void process(float** inputs, float** outputs, int numChannels, int numFrames) override;
    
    void clear();
    
private:
    float delayTimeMs_;
    float feedback_;
    float mix_;
    int sampleRate_;
    int delaySamples_;
    
    struct ChannelBuffer {
        std::vector<float> buffer;
        int writePos = 0;
    };
    
    std::vector<ChannelBuffer> channelBuffers_;
};

class Reverb : public IAudioProcessor {
public:
    Reverb(float roomSize = 0.5f, float damping = 0.5f, float mix = 0.3f);
    
    void setRoomSize(float roomSize);
    void setDamping(float damping);
    void setMix(float mix);
    
    float getRoomSize() const { return roomSize_; }
    float getDamping() const { return damping_; }
    float getMix() const { return mix_; }
    
    void prepare(int sampleRate, int maxBufferSize) override;
    void process(float** inputs, float** outputs, int numChannels, int numFrames) override;
    
    void clear();
    
private:
    float roomSize_;
    float damping_;
    float mix_;
    int sampleRate_;
    
    // Simple comb filter reverb
    struct CombFilter {
        std::vector<float> buffer;
        int bufferSize = 0;
        int writePos = 0;
        float feedback = 0.0f;
        float dampening = 0.0f;
        float filterStore = 0.0f;
        
        float process(float input);
    };
    
    std::vector<std::vector<CombFilter>> combFilters_;  // Per channel
    static const int numCombs = 4;
};

} // namespace OmegaDAW

#endif // OMEGA_DAW_DELAY_H
