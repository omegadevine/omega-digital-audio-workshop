#ifndef OMEGA_DAW_MIXER_CHANNEL_H
#define OMEGA_DAW_MIXER_CHANNEL_H

#include "AudioBuffer.h"
#include <memory>
#include <vector>

namespace omega {

class Effect {
public:
    virtual ~Effect() = default;
    virtual void process(AudioBuffer& buffer) = 0;
    virtual void reset() = 0;
    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;
};

class MixerChannel {
public:
    MixerChannel();
    ~MixerChannel() = default;

    void process(AudioBuffer& inputBuffer, AudioBuffer& outputBuffer);
    
    void addEffect(std::shared_ptr<Effect> effect);
    void removeEffect(int index);
    void clearEffects();
    
    std::shared_ptr<Effect> getEffect(int index);
    int getNumEffects() const { return static_cast<int>(effects_.size()); }
    
    void setPreGain(float gain);
    float getPreGain() const { return preGain_; }
    
    void setPostGain(float gain);
    float getPostGain() const { return postGain_; }

private:
    std::vector<std::shared_ptr<Effect>> effects_;
    float preGain_;
    float postGain_;
};

} // namespace omega

#endif // OMEGA_DAW_MIXER_CHANNEL_H
