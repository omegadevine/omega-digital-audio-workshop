#ifndef OMEGA_DAW_FILTER_H
#define OMEGA_DAW_FILTER_H

#include "AudioEngine.h"
#include <vector>

namespace omega {

enum class FilterType {
    LowPass,
    HighPass,
    BandPass,
    Notch,
    AllPass
};

// Simple biquad filter implementation
class BiquadFilter : public IAudioProcessor {
public:
    BiquadFilter(FilterType type = FilterType::LowPass);
    
    void setType(FilterType type);
    void setFrequency(float frequency);
    void setQ(float q);
    void setGain(float gainDB);
    
    FilterType getType() const { return type_; }
    float getFrequency() const { return frequency_; }
    float getQ() const { return q_; }
    float getGain() const { return gainDB_; }
    
    void prepare(int sampleRate, int maxBufferSize) override;
    void process(float** inputs, float** outputs, int numChannels, int numFrames) override;
    
    void reset();
    
private:
    void updateCoefficients();
    float processSample(float input, int channel);
    
    FilterType type_;
    float frequency_;
    float q_;
    float gainDB_;
    int sampleRate_;
    
    // Biquad coefficients
    float b0_, b1_, b2_;  // Numerator
    float a1_, a2_;       // Denominator (a0 is normalized to 1)
    
    // State variables per channel
    struct ChannelState {
        float x1 = 0.0f;  // Input delayed by 1 sample
        float x2 = 0.0f;  // Input delayed by 2 samples
        float y1 = 0.0f;  // Output delayed by 1 sample
        float y2 = 0.0f;  // Output delayed by 2 samples
    };
    
    std::vector<ChannelState> channelStates_;
    bool coefficientsNeedUpdate_;
};

} // namespace omega

#endif // OMEGA_DAW_FILTER_H
