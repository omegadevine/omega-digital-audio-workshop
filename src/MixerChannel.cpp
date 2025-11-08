#include "MixerChannel.h"

namespace omega {

MixerChannel::MixerChannel()
    : preGain_(1.0f)
    , postGain_(1.0f) {
}

void MixerChannel::process(AudioBuffer& inputBuffer, AudioBuffer& outputBuffer) {
    outputBuffer.copyFrom(inputBuffer);
    
    outputBuffer.applyGain(preGain_);
    
    for (auto& effect : effects_) {
        if (effect && effect->isEnabled()) {
            effect->process(outputBuffer);
        }
    }
    
    outputBuffer.applyGain(postGain_);
}

void MixerChannel::addEffect(std::shared_ptr<Effect> effect) {
    if (effect) {
        effects_.push_back(effect);
    }
}

void MixerChannel::removeEffect(int index) {
    if (index >= 0 && index < static_cast<int>(effects_.size())) {
        effects_.erase(effects_.begin() + index);
    }
}

void MixerChannel::clearEffects() {
    effects_.clear();
}

std::shared_ptr<Effect> MixerChannel::getEffect(int index) {
    if (index >= 0 && index < static_cast<int>(effects_.size())) {
        return effects_[index];
    }
    return nullptr;
}

void MixerChannel::setPreGain(float gain) {
    preGain_ = gain;
}

void MixerChannel::setPostGain(float gain) {
    postGain_ = gain;
}

} // namespace omega
