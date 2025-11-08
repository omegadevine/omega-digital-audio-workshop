#include "Track.h"
#include <algorithm>
#include <cmath>

namespace OmegaDAW {

Track::Track(const std::string& name, TrackType type)
    : name_(name)
    , type_(type)
    , trackIndex_(-1)
    , volume_(1.0f)
    , pan_(0.0f)
    , muted_(false)
    , soloed_(false)
    , recordEnabled_(false)
    , trackBuffer_(2, 512) {
}

void Track::process(AudioBuffer& buffer, int numSamples) {
    if (muted_) {
        return;
    }
    
    trackBuffer_.resize(numSamples);
    
    float leftGain = volume_;
    float rightGain = volume_;
    
    if (pan_ < 0.0f) {
        rightGain *= (1.0f + pan_);
    } else if (pan_ > 0.0f) {
        leftGain *= (1.0f - pan_);
    }
    
    for (int i = 0; i < numSamples; ++i) {
        float left = trackBuffer_.getSample(0, i) * leftGain;
        float right = trackBuffer_.getSample(1, i) * rightGain;
        
        buffer.setSample(0, i, buffer.getSample(0, i) + left);
        buffer.setSample(1, i, buffer.getSample(1, i) + right);
    }
}

void Track::setVolume(float volume) {
    volume_ = std::clamp(volume, 0.0f, 2.0f);
}

void Track::setPan(float pan) {
    pan_ = std::clamp(pan, -1.0f, 1.0f);
}

void Track::setMute(bool mute) {
    muted_ = mute;
}

void Track::setSolo(bool solo) {
    soloed_ = solo;
}

void Track::setRecordEnabled(bool enabled) {
    recordEnabled_ = enabled;
}

} // namespace OmegaDAW
