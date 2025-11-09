#ifndef OMEGA_DAW_OSCILLATOR_H
#define OMEGA_DAW_OSCILLATOR_H

#include "AudioEngine.h"
#include <cmath>

namespace OmegaDAW {

enum class WaveformType {
    Sine,
    Square,
    Saw,
    Triangle,
    Noise
};

class Oscillator : public IAudioProcessor {
public:
    Oscillator(WaveformType type = WaveformType::Sine, float frequency = 440.0f);
    
    void setFrequency(float frequency) { frequency_ = frequency; }
    float getFrequency() const { return frequency_; }
    
    void setAmplitude(float amplitude) { amplitude_ = amplitude; }
    float getAmplitude() const { return amplitude_; }
    
    void setWaveform(WaveformType type) { waveform_ = type; }
    WaveformType getWaveform() const { return waveform_; }
    
    void prepare(int sampleRate, int maxBufferSize) override;
    void process(float** inputs, float** outputs, int numChannels, int numFrames) override;
    
    void reset() { phase_ = 0.0f; }
    
private:
    float generateSample();
    
    WaveformType waveform_;
    float frequency_;
    float amplitude_;
    float phase_;
    float phaseIncrement_;
    int sampleRate_;
};

} // namespace OmegaDAW

#endif // OMEGA_DAW_OSCILLATOR_H
