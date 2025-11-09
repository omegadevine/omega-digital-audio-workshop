#include "AudioBuffer.h"
#include <algorithm>
#include <stdexcept>

namespace OmegaDAW {

AudioBuffer::AudioBuffer(int numChannels, int numSamples)
    : numChannels_(numChannels)
    , numSamples_(numSamples) {
    channelData_.resize(numChannels_);
    for (auto& channel : channelData_) {
        channel.resize(numSamples_, 0.0f);
    }
}

void AudioBuffer::resize(int numSamples) {
    numSamples_ = numSamples;
    for (auto& channel : channelData_) {
        channel.resize(numSamples_, 0.0f);
    }
}

void AudioBuffer::setSize(int numChannels, int numSamples) {
    numChannels_ = numChannels;
    numSamples_ = numSamples;
    channelData_.resize(numChannels_);
    for (auto& channel : channelData_) {
        channel.resize(numSamples_, 0.0f);
    }
}

void AudioBuffer::clear() {
    for (auto& channel : channelData_) {
        std::fill(channel.begin(), channel.end(), 0.0f);
    }
}

float* AudioBuffer::getChannelData(int channel) {
    if (channel < 0 || channel >= numChannels_) {
        return nullptr;
    }
    return channelData_[channel].data();
}

const float* AudioBuffer::getChannelData(int channel) const {
    if (channel < 0 || channel >= numChannels_) {
        return nullptr;
    }
    return channelData_[channel].data();
}

void AudioBuffer::setSample(int channel, int sample, float value) {
    if (channel >= 0 && channel < numChannels_ && sample >= 0 && sample < numSamples_) {
        channelData_[channel][sample] = value;
    }
}

float AudioBuffer::getSample(int channel, int sample) const {
    if (channel >= 0 && channel < numChannels_ && sample >= 0 && sample < numSamples_) {
        return channelData_[channel][sample];
    }
    return 0.0f;
}

void AudioBuffer::copyFrom(const AudioBuffer& other) {
    int channelsToCopy = std::min(numChannels_, other.numChannels_);
    int samplesToCopy = std::min(numSamples_, other.numSamples_);
    
    for (int ch = 0; ch < channelsToCopy; ++ch) {
        std::copy_n(other.channelData_[ch].begin(), samplesToCopy, channelData_[ch].begin());
    }
}

void AudioBuffer::addFrom(const AudioBuffer& other, float gain) {
    int channelsToCopy = std::min(numChannels_, other.numChannels_);
    int samplesToCopy = std::min(numSamples_, other.numSamples_);
    
    for (int ch = 0; ch < channelsToCopy; ++ch) {
        for (int i = 0; i < samplesToCopy; ++i) {
            channelData_[ch][i] += other.channelData_[ch][i] * gain;
        }
    }
}

void AudioBuffer::applyGain(float gain) {
    for (auto& channel : channelData_) {
        for (auto& sample : channel) {
            sample *= gain;
        }
    }
}

void AudioBuffer::applyGainRamp(float startGain, float endGain) {
    if (numSamples_ == 0) return;
    
    float gainDelta = (endGain - startGain) / static_cast<float>(numSamples_);
    
    for (auto& channel : channelData_) {
        float currentGain = startGain;
        for (auto& sample : channel) {
            sample *= currentGain;
            currentGain += gainDelta;
        }
    }
}

} // namespace OmegaDAW
