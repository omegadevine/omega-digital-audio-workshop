#ifndef OMEGA_DAW_ADVANCED_EFFECTS_H
#define OMEGA_DAW_ADVANCED_EFFECTS_H

#include "AudioEngine.h"
#include <vector>
#include <cmath>
#include <algorithm>

namespace OmegaDAW {

// Stereo Enhancer - widens stereo image
class StereoEnhancer : public IAudioProcessor {
public:
    StereoEnhancer(float width = 0.5f);
    
    void process(float** inputs, float** outputs, int numChannels, int numFrames) override;
    void prepare(int sampleRate, int maxBufferSize) override;
    std::string getName() const override { return "Stereo Enhancer"; }
    
    void setWidth(float width); // 0.0 = mono, 1.0 = normal, 2.0 = ultra wide
    float getWidth() const { return width_; }
    
private:
    float width_;
    int sampleRate_;
};

// Multiband Compressor - compress different frequency bands independently
class MultibandCompressor : public IAudioProcessor {
public:
    struct Band {
        float frequency;      // Crossover frequency
        float threshold;      // Compression threshold (dB)
        float ratio;          // Compression ratio
        float attack;         // Attack time (ms)
        float release;        // Release time (ms)
        float makeupGain;     // Output gain (dB)
        
        // Runtime state
        float envelope;
        float gainReduction;
    };
    
    MultibandCompressor();
    
    void process(float** inputs, float** outputs, int numChannels, int numFrames) override;
    void prepare(int sampleRate, int maxBufferSize) override;
    std::string getName() const override { return "Multiband Compressor"; }
    
    void setBand(int index, const Band& band);
    Band& getBand(int index) { return bands_[index]; }
    int getNumBands() const { return 3; }
    
private:
    void processCompression(float* buffer, int numSamples, Band& band);
    
    std::vector<Band> bands_;
    std::vector<std::vector<float>> bandBuffers_;
    int sampleRate_;
    int maxBufferSize_;
};

// Convolution Reverb - high quality reverb using impulse responses
class ConvolutionReverb : public IAudioProcessor {
public:
    ConvolutionReverb();
    
    void process(float** inputs, float** outputs, int numChannels, int numFrames) override;
    void prepare(int sampleRate, int maxBufferSize) override;
    std::string getName() const override { return "Convolution Reverb"; }
    
    bool loadImpulseResponse(const std::string& filename);
    void setDryWetMix(float mix); // 0.0 = dry, 1.0 = wet
    void setPreDelay(float delayMs);
    
private:
    std::vector<float> impulseResponse_;
    std::vector<float> inputHistory_;
    int historyPosition_;
    float dryWet_;
    int preDelayFrames_;
    std::vector<float> preDelayBuffer_;
    int preDelayPosition_;
    int sampleRate_;
};

// Parametric EQ - 4-band parametric equalizer
class ParametricEQ : public IAudioProcessor {
public:
    enum FilterType {
        LOWPASS,
        HIGHPASS,
        PEAK,
        LOWSHELF,
        HIGHSHELF,
        NOTCH
    };
    
    struct EQBand {
        FilterType type;
        float frequency;
        float Q;
        float gainDB;
        bool enabled;
        
        // Filter coefficients
        float b0, b1, b2, a1, a2;
        // Filter state (per channel)
        std::vector<float> x1, x2, y1, y2;
    };
    
    ParametricEQ();
    
    void process(float** inputs, float** outputs, int numChannels, int numFrames) override;
    void prepare(int sampleRate, int maxBufferSize) override;
    std::string getName() const override { return "Parametric EQ"; }
    
    void setBand(int index, FilterType type, float freq, float Q, float gainDB);
    void setBandEnabled(int index, bool enabled);
    EQBand& getBand(int index) { return bands_[index]; }
    int getNumBands() const { return 4; }
    
private:
    void calculateCoefficients(EQBand& band);
    float processSample(float input, EQBand& band, int channel);
    
    std::vector<EQBand> bands_;
    int sampleRate_;
    int numChannels_;
};

// Spectral Gate - frequency-dependent noise gate
class SpectralGate : public IAudioProcessor {
public:
    SpectralGate();
    
    void process(float** inputs, float** outputs, int numChannels, int numFrames) override;
    void prepare(int sampleRate, int maxBufferSize) override;
    std::string getName() const override { return "Spectral Gate"; }
    
    void setThreshold(float thresholdDB);
    void setReduction(float reductionDB);
    void setAttack(float attackMs);
    void setRelease(float releaseMs);
    
private:
    float threshold_;
    float reduction_;
    float attackCoeff_;
    float releaseCoeff_;
    std::vector<float> envelopes_;
    int sampleRate_;
};

// Tube Saturation - analog-style harmonic saturation
class TubeSaturation : public IAudioProcessor {
public:
    TubeSaturation(float drive = 1.0f);
    
    void process(float** inputs, float** outputs, int numChannels, int numFrames) override;
    void prepare(int sampleRate, int maxBufferSize) override;
    std::string getName() const override { return "Tube Saturation"; }
    
    void setDrive(float drive); // 0.0 to 10.0
    void setBias(float bias);   // DC bias for asymmetric saturation
    void setTone(float tone);   // High frequency rolloff
    
    float getDrive() const { return drive_; }
    
private:
    float saturate(float input);
    
    float drive_;
    float bias_;
    float tone_;
    std::vector<float> toneFilterState_;
    int sampleRate_;
};

} // namespace OmegaDAW

#endif // OMEGA_DAW_ADVANCED_EFFECTS_H
