#include "Oscillator.h"
#include <random>
#include <algorithm>

namespace OmegaDAW {

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<float> dis(-1.0f, 1.0f);

Oscillator::Oscillator(WaveformType type, float frequency)
    : waveform_(type)
    , frequency_(frequency)
    , amplitude_(0.5f)
    , phase_(0.0f)
    , phaseIncrement_(0.0f)
    , sampleRate_(48000) {
}

void Oscillator::prepare(int sampleRate, int maxBufferSize) {
    sampleRate_ = sampleRate;
    phaseIncrement_ = frequency_ / static_cast<float>(sampleRate_);
}

void Oscillator::process(float** inputs, float** outputs, int numChannels, int numFrames) {
    (void)inputs;  // Not used for oscillator
    
    for (int frame = 0; frame < numFrames; ++frame) {
        float sample = generateSample();
        
        // Output to all channels
        for (int ch = 0; ch < numChannels; ++ch) {
            outputs[ch][frame] += sample * amplitude_;
        }
        
        // Advance phase
        phase_ += phaseIncrement_;
        if (phase_ >= 1.0f) {
            phase_ -= 1.0f;
        }
    }
}

float Oscillator::generateSample() {
    const float pi = 3.14159265358979323846f;
    const float twoPi = 2.0f * pi;
    
    switch (waveform_) {
        case WaveformType::Sine:
            return std::sin(twoPi * phase_);
            
        case WaveformType::Square:
            return (phase_ < 0.5f) ? 1.0f : -1.0f;
            
        case WaveformType::Saw:
            return 2.0f * phase_ - 1.0f;
            
        case WaveformType::Triangle:
            if (phase_ < 0.5f) {
                return 4.0f * phase_ - 1.0f;
            } else {
                return -4.0f * phase_ + 3.0f;
            }
            
        case WaveformType::Noise:
            return dis(gen);
            
        default:
            return 0.0f;
    }
}

} // namespace OmegaDAW
