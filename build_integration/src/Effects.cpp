#include "Effects.h"
#include <algorithm>
#include <cmath>

namespace OmegaDAW {

// ============================================================================
// Delay Implementation
// ============================================================================

Delay::Delay(float delayTimeMs, float feedback, float mix)
    : delayTimeMs_(delayTimeMs)
    , feedback_(feedback)
    , mix_(mix)
    , sampleRate_(48000)
    , delaySamples_(0) {
}

void Delay::setDelayTime(float delayTimeMs) {
    delayTimeMs_ = std::max(0.0f, std::min(delayTimeMs, 5000.0f));
    delaySamples_ = static_cast<int>(delayTimeMs_ * sampleRate_ / 1000.0f);
    
    // Resize buffers if needed
    for (auto& channelBuffer : channelBuffers_) {
        if (static_cast<int>(channelBuffer.buffer.size()) < delaySamples_) {
            channelBuffer.buffer.resize(delaySamples_ + 1, 0.0f);
        }
    }
}

void Delay::setFeedback(float feedback) {
    feedback_ = std::max(0.0f, std::min(feedback, 0.95f));
}

void Delay::setMix(float mix) {
    mix_ = std::max(0.0f, std::min(mix, 1.0f));
}

void Delay::prepare(int sampleRate, int maxBufferSize) {
    sampleRate_ = sampleRate;
    delaySamples_ = static_cast<int>(delayTimeMs_ * sampleRate_ / 1000.0f);
    
    channelBuffers_.clear();
    channelBuffers_.resize(8);  // Support up to 8 channels
    
    for (auto& channelBuffer : channelBuffers_) {
        channelBuffer.buffer.resize(delaySamples_ + 1, 0.0f);
        channelBuffer.writePos = 0;
    }
}

void Delay::process(float** inputs, float** outputs, int numChannels, int numFrames) {
    if (delaySamples_ <= 0) {
        return;  // Pass through
    }
    
    for (int ch = 0; ch < numChannels; ++ch) {
        if (ch >= static_cast<int>(channelBuffers_.size())) break;
        
        ChannelBuffer& buffer = channelBuffers_[ch];
        
        for (int frame = 0; frame < numFrames; ++frame) {
            float input = (inputs && inputs[ch]) ? inputs[ch][frame] : outputs[ch][frame];
            
            // Calculate read position
            int readPos = buffer.writePos - delaySamples_;
            if (readPos < 0) {
                readPos += buffer.buffer.size();
            }
            
            // Read delayed sample
            float delayedSample = buffer.buffer[readPos];
            
            // Write input + feedback to buffer
            buffer.buffer[buffer.writePos] = input + (delayedSample * feedback_);
            
            // Mix dry and wet signals
            outputs[ch][frame] = input * (1.0f - mix_) + delayedSample * mix_;
            
            // Advance write position
            buffer.writePos++;
            if (buffer.writePos >= static_cast<int>(buffer.buffer.size())) {
                buffer.writePos = 0;
            }
        }
    }
}

void Delay::clear() {
    for (auto& channelBuffer : channelBuffers_) {
        std::fill(channelBuffer.buffer.begin(), channelBuffer.buffer.end(), 0.0f);
        channelBuffer.writePos = 0;
    }
}

// ============================================================================
// Reverb Implementation
// ============================================================================

Reverb::Reverb(float roomSize, float damping, float mix)
    : roomSize_(roomSize)
    , damping_(damping)
    , mix_(mix)
    , sampleRate_(48000) {
}

void Reverb::setRoomSize(float roomSize) {
    roomSize_ = std::max(0.0f, std::min(roomSize, 1.0f));
    
    // Update comb filter feedback
    for (auto& channelFilters : combFilters_) {
        for (auto& comb : channelFilters) {
            comb.feedback = 0.7f + (roomSize_ * 0.28f);
        }
    }
}

void Reverb::setDamping(float damping) {
    damping_ = std::max(0.0f, std::min(damping, 1.0f));
    
    // Update comb filter dampening
    for (auto& channelFilters : combFilters_) {
        for (auto& comb : channelFilters) {
            comb.dampening = damping_;
        }
    }
}

void Reverb::setMix(float mix) {
    mix_ = std::max(0.0f, std::min(mix, 1.0f));
}

void Reverb::prepare(int sampleRate, int maxBufferSize) {
    sampleRate_ = sampleRate;
    
    // Comb filter delays (in samples) - classic Freeverb values scaled
    const int combDelays[numCombs] = {1116, 1188, 1277, 1356};
    
    combFilters_.clear();
    combFilters_.resize(8);  // Support up to 8 channels
    
    for (auto& channelFilters : combFilters_) {
        channelFilters.resize(numCombs);
        
        for (int i = 0; i < numCombs; ++i) {
            CombFilter& comb = channelFilters[i];
            comb.bufferSize = combDelays[i];
            comb.buffer.resize(comb.bufferSize, 0.0f);
            comb.writePos = 0;
            comb.feedback = 0.7f + (roomSize_ * 0.28f);
            comb.dampening = damping_;
            comb.filterStore = 0.0f;
        }
    }
}

void Reverb::process(float** inputs, float** outputs, int numChannels, int numFrames) {
    for (int ch = 0; ch < numChannels; ++ch) {
        if (ch >= static_cast<int>(combFilters_.size())) break;
        
        auto& filters = combFilters_[ch];
        
        for (int frame = 0; frame < numFrames; ++frame) {
            float input = (inputs && inputs[ch]) ? inputs[ch][frame] : outputs[ch][frame];
            float reverbSample = 0.0f;
            
            // Process through all comb filters and sum
            for (auto& comb : filters) {
                reverbSample += comb.process(input);
            }
            
            // Average the comb filter outputs
            reverbSample /= static_cast<float>(numCombs);
            
            // Mix dry and wet
            outputs[ch][frame] = input * (1.0f - mix_) + reverbSample * mix_;
        }
    }
}

void Reverb::clear() {
    for (auto& channelFilters : combFilters_) {
        for (auto& comb : channelFilters) {
            std::fill(comb.buffer.begin(), comb.buffer.end(), 0.0f);
            comb.writePos = 0;
            comb.filterStore = 0.0f;
        }
    }
}

float Reverb::CombFilter::process(float input) {
    if (bufferSize == 0) return 0.0f;
    
    // Read from buffer
    float output = buffer[writePos];
    
    // Simple one-pole lowpass filter for dampening
    filterStore = (output * (1.0f - dampening)) + (filterStore * dampening);
    
    // Write input + feedback to buffer
    buffer[writePos] = input + (filterStore * feedback);
    
    // Advance write position
    writePos++;
    if (writePos >= bufferSize) {
        writePos = 0;
    }
    
    return output;
}

} // namespace OmegaDAW
