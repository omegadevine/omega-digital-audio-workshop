#include "Filter.h"
#include <cmath>
#include <algorithm>

namespace OmegaDAW {

BiquadFilter::BiquadFilter(FilterType type)
    : type_(type)
    , frequency_(1000.0f)
    , q_(0.707f)  // Butterworth Q
    , gainDB_(0.0f)
    , sampleRate_(48000)
    , b0_(1.0f), b1_(0.0f), b2_(0.0f)
    , a1_(0.0f), a2_(0.0f)
    , coefficientsNeedUpdate_(true) {
}

void BiquadFilter::setType(FilterType type) {
    if (type_ != type) {
        type_ = type;
        coefficientsNeedUpdate_ = true;
    }
}

void BiquadFilter::setFrequency(float frequency) {
    if (frequency_ != frequency) {
        frequency_ = std::max(20.0f, std::min(frequency, static_cast<float>(sampleRate_) * 0.49f));
        coefficientsNeedUpdate_ = true;
    }
}

void BiquadFilter::setQ(float q) {
    if (q_ != q) {
        q_ = std::max(0.01f, std::min(q, 20.0f));
        coefficientsNeedUpdate_ = true;
    }
}

void BiquadFilter::setGain(float gainDB) {
    if (gainDB_ != gainDB) {
        gainDB_ = gainDB;
        coefficientsNeedUpdate_ = true;
    }
}

void BiquadFilter::prepare(int sampleRate, int maxBufferSize) {
    sampleRate_ = sampleRate;
    channelStates_.clear();
    channelStates_.resize(8);  // Support up to 8 channels
    coefficientsNeedUpdate_ = true;
}

void BiquadFilter::process(float** inputs, float** outputs, int numChannels, int numFrames) {
    if (coefficientsNeedUpdate_) {
        updateCoefficients();
        coefficientsNeedUpdate_ = false;
    }
    
    for (int ch = 0; ch < numChannels; ++ch) {
        for (int frame = 0; frame < numFrames; ++frame) {
            float input = (inputs && inputs[ch]) ? inputs[ch][frame] : outputs[ch][frame];
            outputs[ch][frame] = processSample(input, ch);
        }
    }
}

void BiquadFilter::reset() {
    for (auto& state : channelStates_) {
        state.x1 = state.x2 = state.y1 = state.y2 = 0.0f;
    }
}

void BiquadFilter::updateCoefficients() {
    const float pi = 3.14159265358979323846f;
    const float omega = 2.0f * pi * frequency_ / static_cast<float>(sampleRate_);
    const float cosw = std::cos(omega);
    const float sinw = std::sin(omega);
    const float alpha = sinw / (2.0f * q_);
    const float A = std::pow(10.0f, gainDB_ / 40.0f);
    
    float a0, a1, a2, b0, b1, b2;
    
    switch (type_) {
        case FilterType::LowPass:
            b0 = (1.0f - cosw) / 2.0f;
            b1 = 1.0f - cosw;
            b2 = (1.0f - cosw) / 2.0f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cosw;
            a2 = 1.0f - alpha;
            break;
            
        case FilterType::HighPass:
            b0 = (1.0f + cosw) / 2.0f;
            b1 = -(1.0f + cosw);
            b2 = (1.0f + cosw) / 2.0f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cosw;
            a2 = 1.0f - alpha;
            break;
            
        case FilterType::BandPass:
            b0 = alpha;
            b1 = 0.0f;
            b2 = -alpha;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cosw;
            a2 = 1.0f - alpha;
            break;
            
        case FilterType::Notch:
            b0 = 1.0f;
            b1 = -2.0f * cosw;
            b2 = 1.0f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cosw;
            a2 = 1.0f - alpha;
            break;
            
        case FilterType::AllPass:
            b0 = 1.0f - alpha;
            b1 = -2.0f * cosw;
            b2 = 1.0f + alpha;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cosw;
            a2 = 1.0f - alpha;
            break;
            
        default:
            b0 = 1.0f; b1 = 0.0f; b2 = 0.0f;
            a0 = 1.0f; a1 = 0.0f; a2 = 0.0f;
            break;
    }
    
    // Normalize coefficients
    b0_ = b0 / a0;
    b1_ = b1 / a0;
    b2_ = b2 / a0;
    a1_ = a1 / a0;
    a2_ = a2 / a0;
}

float BiquadFilter::processSample(float input, int channel) {
    if (channel >= static_cast<int>(channelStates_.size())) {
        return input;
    }
    
    ChannelState& state = channelStates_[channel];
    
    // Direct Form II Transposed implementation
    float output = b0_ * input + b1_ * state.x1 + b2_ * state.x2
                 - a1_ * state.y1 - a2_ * state.y2;
    
    // Update state
    state.x2 = state.x1;
    state.x1 = input;
    state.y2 = state.y1;
    state.y1 = output;
    
    return output;
}

} // namespace OmegaDAW
