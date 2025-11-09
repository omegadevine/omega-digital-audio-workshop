#pragma once

#include "Plugin.h"
#include <cmath>

namespace OmegaDAW {

class GainPlugin : public Plugin {
public:
    GainPlugin();
    void initialize(int sampleRate, int maxBufferSize) override;
    void process(float** inputs, float** outputs, int numChannels, int numSamples) override;
    void reset() override;
};

class DelayPlugin : public Plugin {
public:
    DelayPlugin();
    void initialize(int sampleRate, int maxBufferSize) override;
    void process(float** inputs, float** outputs, int numChannels, int numSamples) override;
    void reset() override;

private:
    std::vector<std::vector<float>> delayBuffer;
    size_t writePosition;
};

class ReverbPlugin : public Plugin {
public:
    ReverbPlugin();
    void initialize(int sampleRate, int maxBufferSize) override;
    void process(float** inputs, float** outputs, int numChannels, int numSamples) override;
    void reset() override;

private:
    std::vector<std::vector<float>> combBuffers;
    std::vector<size_t> combPositions;
    std::vector<std::vector<float>> allpassBuffers;
    std::vector<size_t> allpassPositions;
};

class CompressorPlugin : public Plugin {
public:
    CompressorPlugin();
    void initialize(int sampleRate, int maxBufferSize) override;
    void process(float** inputs, float** outputs, int numChannels, int numSamples) override;
    void reset() override;

private:
    float envelope;
};

class EQPlugin : public Plugin {
public:
    EQPlugin();
    void initialize(int sampleRate, int maxBufferSize) override;
    void process(float** inputs, float** outputs, int numChannels, int numSamples) override;
    void reset() override;

private:
    struct BiquadFilter {
        float b0, b1, b2, a1, a2;
        float x1, x2, y1, y2;
    };
    
    std::vector<std::vector<BiquadFilter>> filters;
    void calculateBiquadCoeffs(BiquadFilter& filter, float freq, float Q, float gain);
};

} // namespace OmegaDAW
