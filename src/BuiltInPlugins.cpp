#include "BuiltInPlugins.h"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace OmegaDAW {

// Gain Plugin
GainPlugin::GainPlugin() : Plugin("Gain", PluginType::Effect) {
    PluginParameter gainParam;
    gainParam.id = "gain";
    gainParam.name = "Gain";
    gainParam.value = 1.0f;
    gainParam.minValue = 0.0f;
    gainParam.maxValue = 2.0f;
    gainParam.defaultValue = 1.0f;
    gainParam.unit = "x";
    gainParam.isAutomatable = true;
    addParameter(gainParam);
}

void GainPlugin::initialize(int sampleRate, int maxBufferSize) {
    this->sampleRate = sampleRate;
    this->maxBufferSize = maxBufferSize;
}

void GainPlugin::process(float** inputs, float** outputs, int numChannels, int numSamples) {
    float gain = getParameter("gain");
    
    for (int ch = 0; ch < numChannels; ++ch) {
        for (int i = 0; i < numSamples; ++i) {
            outputs[ch][i] = inputs[ch][i] * gain;
        }
    }
}

void GainPlugin::reset() {
}

// Delay Plugin
DelayPlugin::DelayPlugin() : Plugin("Delay", PluginType::Effect), writePosition(0) {
    PluginParameter delayTimeParam;
    delayTimeParam.id = "delaytime";
    delayTimeParam.name = "Delay Time";
    delayTimeParam.value = 0.5f;
    delayTimeParam.minValue = 0.0f;
    delayTimeParam.maxValue = 2.0f;
    delayTimeParam.defaultValue = 0.5f;
    delayTimeParam.unit = "s";
    delayTimeParam.isAutomatable = true;
    addParameter(delayTimeParam);
    
    PluginParameter feedbackParam;
    feedbackParam.id = "feedback";
    feedbackParam.name = "Feedback";
    feedbackParam.value = 0.3f;
    feedbackParam.minValue = 0.0f;
    feedbackParam.maxValue = 0.95f;
    feedbackParam.defaultValue = 0.3f;
    feedbackParam.unit = "";
    feedbackParam.isAutomatable = true;
    addParameter(feedbackParam);
    
    PluginParameter mixParam;
    mixParam.id = "mix";
    mixParam.name = "Mix";
    mixParam.value = 0.5f;
    mixParam.minValue = 0.0f;
    mixParam.maxValue = 1.0f;
    mixParam.defaultValue = 0.5f;
    mixParam.unit = "";
    mixParam.isAutomatable = true;
    addParameter(mixParam);
}

void DelayPlugin::initialize(int sampleRate, int maxBufferSize) {
    this->sampleRate = sampleRate;
    this->maxBufferSize = maxBufferSize;
    
    size_t maxDelaySamples = static_cast<size_t>(sampleRate * 2.0f);
    delayBuffer.clear();
    delayBuffer.resize(2, std::vector<float>(maxDelaySamples, 0.0f));
    writePosition = 0;
}

void DelayPlugin::process(float** inputs, float** outputs, int numChannels, int numSamples) {
    float delayTime = getParameter("delaytime");
    float feedback = getParameter("feedback");
    float mix = getParameter("mix");
    
    size_t delaySamples = static_cast<size_t>(delayTime * sampleRate);
    size_t bufferSize = delayBuffer[0].size();
    
    for (int ch = 0; ch < std::min(numChannels, 2); ++ch) {
        for (int i = 0; i < numSamples; ++i) {
            size_t readPos = (writePosition + bufferSize - delaySamples) % bufferSize;
            float delayedSample = delayBuffer[ch][readPos];
            
            delayBuffer[ch][writePosition] = inputs[ch][i] + delayedSample * feedback;
            outputs[ch][i] = inputs[ch][i] * (1.0f - mix) + delayedSample * mix;
            
            writePosition = (writePosition + 1) % bufferSize;
        }
    }
}

void DelayPlugin::reset() {
    for (auto& buffer : delayBuffer) {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
    }
    writePosition = 0;
}

// Reverb Plugin
ReverbPlugin::ReverbPlugin() : Plugin("Reverb", PluginType::Effect) {
    PluginParameter roomSizeParam;
    roomSizeParam.id = "roomsize";
    roomSizeParam.name = "Room Size";
    roomSizeParam.value = 0.5f;
    roomSizeParam.minValue = 0.0f;
    roomSizeParam.maxValue = 1.0f;
    roomSizeParam.defaultValue = 0.5f;
    roomSizeParam.unit = "";
    roomSizeParam.isAutomatable = true;
    addParameter(roomSizeParam);
    
    PluginParameter dampingParam;
    dampingParam.id = "damping";
    dampingParam.name = "Damping";
    dampingParam.value = 0.5f;
    dampingParam.minValue = 0.0f;
    dampingParam.maxValue = 1.0f;
    dampingParam.defaultValue = 0.5f;
    dampingParam.unit = "";
    dampingParam.isAutomatable = true;
    addParameter(dampingParam);
    
    PluginParameter mixParam;
    mixParam.id = "mix";
    mixParam.name = "Mix";
    mixParam.value = 0.3f;
    mixParam.minValue = 0.0f;
    mixParam.maxValue = 1.0f;
    mixParam.defaultValue = 0.3f;
    mixParam.unit = "";
    mixParam.isAutomatable = true;
    addParameter(mixParam);
}

void ReverbPlugin::initialize(int sampleRate, int maxBufferSize) {
    this->sampleRate = sampleRate;
    this->maxBufferSize = maxBufferSize;
    
    const int combSizes[] = {1557, 1617, 1491, 1422, 1277, 1356, 1188, 1116};
    const int allpassSizes[] = {225, 556, 441, 341};
    
    combBuffers.resize(8);
    combPositions.resize(8, 0);
    for (int i = 0; i < 8; ++i) {
        combBuffers[i].resize(combSizes[i], 0.0f);
    }
    
    allpassBuffers.resize(4);
    allpassPositions.resize(4, 0);
    for (int i = 0; i < 4; ++i) {
        allpassBuffers[i].resize(allpassSizes[i], 0.0f);
    }
}

void ReverbPlugin::process(float** inputs, float** outputs, int numChannels, int numSamples) {
    float roomSize = getParameter("roomsize");
    float damping = getParameter("damping");
    float mix = getParameter("mix");
    
    for (int i = 0; i < numSamples; ++i) {
        float inputSample = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch) {
            inputSample += inputs[ch][i];
        }
        inputSample /= numChannels;
        
        float reverbSample = 0.0f;
        for (size_t c = 0; c < combBuffers.size(); ++c) {
            size_t pos = combPositions[c];
            float combOut = combBuffers[c][pos];
            combBuffers[c][pos] = inputSample + combOut * roomSize * (1.0f - damping);
            reverbSample += combOut;
            combPositions[c] = (pos + 1) % combBuffers[c].size();
        }
        reverbSample /= combBuffers.size();
        
        for (size_t a = 0; a < allpassBuffers.size(); ++a) {
            size_t pos = allpassPositions[a];
            float allpassOut = allpassBuffers[a][pos];
            allpassBuffers[a][pos] = reverbSample + allpassOut * 0.5f;
            reverbSample = allpassOut - reverbSample * 0.5f;
            allpassPositions[a] = (pos + 1) % allpassBuffers[a].size();
        }
        
        for (int ch = 0; ch < numChannels; ++ch) {
            outputs[ch][i] = inputs[ch][i] * (1.0f - mix) + reverbSample * mix;
        }
    }
}

void ReverbPlugin::reset() {
    for (auto& buffer : combBuffers) {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
    }
    std::fill(combPositions.begin(), combPositions.end(), 0);
    
    for (auto& buffer : allpassBuffers) {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
    }
    std::fill(allpassPositions.begin(), allpassPositions.end(), 0);
}

// Compressor Plugin
CompressorPlugin::CompressorPlugin() : Plugin("Compressor", PluginType::Effect), envelope(0.0f) {
    PluginParameter thresholdParam;
    thresholdParam.id = "threshold";
    thresholdParam.name = "Threshold";
    thresholdParam.value = -20.0f;
    thresholdParam.minValue = -60.0f;
    thresholdParam.maxValue = 0.0f;
    thresholdParam.defaultValue = -20.0f;
    thresholdParam.unit = "dB";
    thresholdParam.isAutomatable = true;
    addParameter(thresholdParam);
    
    PluginParameter ratioParam;
    ratioParam.id = "ratio";
    ratioParam.name = "Ratio";
    ratioParam.value = 4.0f;
    ratioParam.minValue = 1.0f;
    ratioParam.maxValue = 20.0f;
    ratioParam.defaultValue = 4.0f;
    ratioParam.unit = ":1";
    ratioParam.isAutomatable = true;
    addParameter(ratioParam);
    
    PluginParameter attackParam;
    attackParam.id = "attack";
    attackParam.name = "Attack";
    attackParam.value = 0.01f;
    attackParam.minValue = 0.001f;
    attackParam.maxValue = 0.1f;
    attackParam.defaultValue = 0.01f;
    attackParam.unit = "s";
    attackParam.isAutomatable = true;
    addParameter(attackParam);
    
    PluginParameter releaseParam;
    releaseParam.id = "release";
    releaseParam.name = "Release";
    releaseParam.value = 0.1f;
    releaseParam.minValue = 0.01f;
    releaseParam.maxValue = 1.0f;
    releaseParam.defaultValue = 0.1f;
    releaseParam.unit = "s";
    releaseParam.isAutomatable = true;
    addParameter(releaseParam);
}

void CompressorPlugin::initialize(int sampleRate, int maxBufferSize) {
    this->sampleRate = sampleRate;
    this->maxBufferSize = maxBufferSize;
    envelope = 0.0f;
}

void CompressorPlugin::process(float** inputs, float** outputs, int numChannels, int numSamples) {
    float threshold = getParameter("threshold");
    float ratio = getParameter("ratio");
    float attack = getParameter("attack");
    float release = getParameter("release");
    
    float thresholdLin = std::pow(10.0f, threshold / 20.0f);
    float attackCoeff = std::exp(-1.0f / (attack * sampleRate));
    float releaseCoeff = std::exp(-1.0f / (release * sampleRate));
    
    for (int i = 0; i < numSamples; ++i) {
        float maxSample = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch) {
            maxSample = std::max(maxSample, std::abs(inputs[ch][i]));
        }
        
        float coeff = (maxSample > envelope) ? attackCoeff : releaseCoeff;
        envelope = maxSample + coeff * (envelope - maxSample);
        
        float gain = 1.0f;
        if (envelope > thresholdLin) {
            float overDb = 20.0f * std::log10(envelope / thresholdLin);
            float gainReduction = overDb * (1.0f - 1.0f / ratio);
            gain = std::pow(10.0f, -gainReduction / 20.0f);
        }
        
        for (int ch = 0; ch < numChannels; ++ch) {
            outputs[ch][i] = inputs[ch][i] * gain;
        }
    }
}

void CompressorPlugin::reset() {
    envelope = 0.0f;
}

// EQ Plugin
EQPlugin::EQPlugin() : Plugin("EQ", PluginType::Effect) {
    const char* bands[] = {"low", "mid", "high"};
    const float freqs[] = {100.0f, 1000.0f, 10000.0f};
    
    for (int i = 0; i < 3; ++i) {
        PluginParameter gainParam;
        gainParam.id = std::string(bands[i]) + "_gain";
        gainParam.name = std::string(bands[i]) + " Gain";
        gainParam.value = 0.0f;
        gainParam.minValue = -12.0f;
        gainParam.maxValue = 12.0f;
        gainParam.defaultValue = 0.0f;
        gainParam.unit = "dB";
        gainParam.isAutomatable = true;
        addParameter(gainParam);
        
        PluginParameter freqParam;
        freqParam.id = std::string(bands[i]) + "_freq";
        freqParam.name = std::string(bands[i]) + " Frequency";
        freqParam.value = freqs[i];
        freqParam.minValue = 20.0f;
        freqParam.maxValue = 20000.0f;
        freqParam.defaultValue = freqs[i];
        freqParam.unit = "Hz";
        freqParam.isAutomatable = true;
        addParameter(freqParam);
    }
}

void EQPlugin::initialize(int sampleRate, int maxBufferSize) {
    this->sampleRate = sampleRate;
    this->maxBufferSize = maxBufferSize;
    
    filters.resize(2);
    for (int ch = 0; ch < 2; ++ch) {
        filters[ch].resize(3);
        for (int band = 0; band < 3; ++band) {
            filters[ch][band] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        }
    }
}

void EQPlugin::calculateBiquadCoeffs(BiquadFilter& filter, float freq, float Q, float gain) {
    float K = std::tan(M_PI * freq / sampleRate);
    float V = std::pow(10.0f, std::abs(gain) / 20.0f);
    
    if (gain >= 0.0f) {
        filter.b0 = (1.0f + V * K / Q + K * K) / (1.0f + K / Q + K * K);
        filter.b1 = 2.0f * (K * K - 1.0f) / (1.0f + K / Q + K * K);
        filter.b2 = (1.0f - V * K / Q + K * K) / (1.0f + K / Q + K * K);
        filter.a1 = filter.b1;
        filter.a2 = (1.0f - K / Q + K * K) / (1.0f + K / Q + K * K);
    } else {
        filter.b0 = (1.0f + K / Q + K * K) / (1.0f + K / (V * Q) + K * K);
        filter.b1 = 2.0f * (K * K - 1.0f) / (1.0f + K / (V * Q) + K * K);
        filter.b2 = (1.0f - K / Q + K * K) / (1.0f + K / (V * Q) + K * K);
        filter.a1 = filter.b1;
        filter.a2 = (1.0f - K / (V * Q) + K * K) / (1.0f + K / (V * Q) + K * K);
    }
}

void EQPlugin::process(float** inputs, float** outputs, int numChannels, int numSamples) {
    const char* bands[] = {"low", "mid", "high"};
    
    for (int band = 0; band < 3; ++band) {
        float gain = getParameter(std::string(bands[band]) + "_gain");
        float freq = getParameter(std::string(bands[band]) + "_freq");
        
        for (int ch = 0; ch < std::min(numChannels, 2); ++ch) {
            calculateBiquadCoeffs(filters[ch][band], freq, 0.707f, gain);
        }
    }
    
    for (int ch = 0; ch < std::min(numChannels, 2); ++ch) {
        for (int i = 0; i < numSamples; ++i) {
            float sample = inputs[ch][i];
            
            for (int band = 0; band < 3; ++band) {
                auto& f = filters[ch][band];
                float output = f.b0 * sample + f.b1 * f.x1 + f.b2 * f.x2 
                             - f.a1 * f.y1 - f.a2 * f.y2;
                
                f.x2 = f.x1;
                f.x1 = sample;
                f.y2 = f.y1;
                f.y1 = output;
                
                sample = output;
            }
            
            outputs[ch][i] = sample;
        }
    }
}

void EQPlugin::reset() {
    for (auto& channelFilters : filters) {
        for (auto& filter : channelFilters) {
            filter.x1 = filter.x2 = filter.y1 = filter.y2 = 0.0f;
        }
    }
}

} // namespace OmegaDAW
