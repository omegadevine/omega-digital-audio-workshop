#include "AdvancedEffects.h"
#include <iostream>
#include <cmath>
#include <algorithm>

namespace OmegaDAW {

// ===== Stereo Enhancer =====

StereoEnhancer::StereoEnhancer(float width)
    : width_(width)
    , sampleRate_(48000) {
}

void StereoEnhancer::prepare(int sampleRate, int maxBufferSize) {
    sampleRate_ = sampleRate;
}

void StereoEnhancer::setWidth(float width) {
    width_ = std::max(0.0f, std::min(2.0f, width));
}

void StereoEnhancer::process(float** inputs, float** outputs, int numChannels, int numFrames) {
    if (numChannels < 2 || isBypassed()) {
        // Mono or bypassed - just copy
        for (int ch = 0; ch < numChannels; ++ch) {
            for (int i = 0; i < numFrames; ++i) {
                outputs[ch][i] = outputs[ch][i];
            }
        }
        return;
    }
    
    for (int i = 0; i < numFrames; ++i) {
        float left = outputs[0][i];
        float right = outputs[1][i];
        
        // Mid-side processing
        float mid = (left + right) * 0.5f;
        float side = (left - right) * 0.5f;
        
        // Enhance stereo width
        side *= (1.0f + width_);
        
        // Convert back to left-right
        outputs[0][i] = mid + side;
        outputs[1][i] = mid - side;
    }
}

// ===== Multiband Compressor =====

MultibandCompressor::MultibandCompressor()
    : sampleRate_(48000)
    , maxBufferSize_(0) {
    
    // Initialize 3 bands: Low, Mid, High
    bands_.resize(3);
    
    // Low band (20-200 Hz)
    bands_[0].frequency = 200.0f;
    bands_[0].threshold = -20.0f;
    bands_[0].ratio = 3.0f;
    bands_[0].attack = 10.0f;
    bands_[0].release = 100.0f;
    bands_[0].makeupGain = 0.0f;
    bands_[0].envelope = 0.0f;
    bands_[0].gainReduction = 0.0f;
    
    // Mid band (200-2000 Hz)
    bands_[1].frequency = 2000.0f;
    bands_[1].threshold = -15.0f;
    bands_[1].ratio = 2.5f;
    bands_[1].attack = 5.0f;
    bands_[1].release = 80.0f;
    bands_[1].makeupGain = 0.0f;
    bands_[1].envelope = 0.0f;
    bands_[1].gainReduction = 0.0f;
    
    // High band (2000+ Hz)
    bands_[2].frequency = 20000.0f;
    bands_[2].threshold = -10.0f;
    bands_[2].ratio = 2.0f;
    bands_[2].attack = 3.0f;
    bands_[2].release = 60.0f;
    bands_[2].makeupGain = 0.0f;
    bands_[2].envelope = 0.0f;
    bands_[2].gainReduction = 0.0f;
}

void MultibandCompressor::prepare(int sampleRate, int maxBufferSize) {
    sampleRate_ = sampleRate;
    maxBufferSize_ = maxBufferSize;
    
    bandBuffers_.resize(bands_.size());
    for (auto& buffer : bandBuffers_) {
        buffer.resize(maxBufferSize, 0.0f);
    }
}

void MultibandCompressor::setBand(int index, const Band& band) {
    if (index >= 0 && index < static_cast<int>(bands_.size())) {
        bands_[index] = band;
    }
}

void MultibandCompressor::processCompression(float* buffer, int numSamples, Band& band) {
    float attackCoeff = exp(-1.0f / (band.attack * sampleRate_ * 0.001f));
    float releaseCoeff = exp(-1.0f / (band.release * sampleRate_ * 0.001f));
    float thresholdLin = pow(10.0f, band.threshold / 20.0f);
    float makeupGainLin = pow(10.0f, band.makeupGain / 20.0f);
    
    for (int i = 0; i < numSamples; ++i) {
        float input = std::abs(buffer[i]);
        
        // Envelope follower
        float coeff = (input > band.envelope) ? attackCoeff : releaseCoeff;
        band.envelope = input + coeff * (band.envelope - input);
        
        // Calculate gain reduction
        float gainReduction = 1.0f;
        if (band.envelope > thresholdLin) {
            gainReduction = thresholdLin / band.envelope;
            gainReduction = pow(gainReduction, (band.ratio - 1.0f) / band.ratio);
        }
        
        band.gainReduction = gainReduction;
        
        // Apply compression and makeup gain
        buffer[i] *= gainReduction * makeupGainLin;
    }
}

void MultibandCompressor::process(float** inputs, float** outputs, int numChannels, int numFrames) {
    if (isBypassed()) return;
    
    // Simple implementation: process each channel through all bands
    // In a real multiband compressor, we'd use crossover filters to split frequencies
    for (int ch = 0; ch < numChannels; ++ch) {
        // Copy to temporary buffer for each band
        for (size_t band = 0; band < bands_.size(); ++band) {
            std::copy(outputs[ch], outputs[ch] + numFrames, bandBuffers_[band].begin());
            processCompression(bandBuffers_[band].data(), numFrames, bands_[band]);
        }
        
        // Mix bands back together (simplified - should use proper crossovers)
        for (int i = 0; i < numFrames; ++i) {
            float sum = 0.0f;
            for (size_t band = 0; band < bands_.size(); ++band) {
                sum += bandBuffers_[band][i];
            }
            outputs[ch][i] = sum / bands_.size();
        }
    }
}

// ===== Parametric EQ =====

ParametricEQ::ParametricEQ()
    : sampleRate_(48000)
    , numChannels_(2) {
    
    bands_.resize(4);
    
    // Low shelf at 80 Hz
    bands_[0].type = LOWSHELF;
    bands_[0].frequency = 80.0f;
    bands_[0].Q = 0.707f;
    bands_[0].gainDB = 0.0f;
    bands_[0].enabled = true;
    
    // Peak at 500 Hz
    bands_[1].type = PEAK;
    bands_[1].frequency = 500.0f;
    bands_[1].Q = 1.0f;
    bands_[1].gainDB = 0.0f;
    bands_[1].enabled = true;
    
    // Peak at 2000 Hz
    bands_[2].type = PEAK;
    bands_[2].frequency = 2000.0f;
    bands_[2].Q = 1.0f;
    bands_[2].gainDB = 0.0f;
    bands_[2].enabled = true;
    
    // High shelf at 8000 Hz
    bands_[3].type = HIGHSHELF;
    bands_[3].frequency = 8000.0f;
    bands_[3].Q = 0.707f;
    bands_[3].gainDB = 0.0f;
    bands_[3].enabled = true;
}

void ParametricEQ::prepare(int sampleRate, int maxBufferSize) {
    sampleRate_ = sampleRate;
    numChannels_ = 2;
    
    // Initialize filter state for each band
    for (auto& band : bands_) {
        band.x1.resize(numChannels_, 0.0f);
        band.x2.resize(numChannels_, 0.0f);
        band.y1.resize(numChannels_, 0.0f);
        band.y2.resize(numChannels_, 0.0f);
        calculateCoefficients(band);
    }
}

void ParametricEQ::setBand(int index, FilterType type, float freq, float Q, float gainDB) {
    if (index >= 0 && index < static_cast<int>(bands_.size())) {
        bands_[index].type = type;
        bands_[index].frequency = freq;
        bands_[index].Q = Q;
        bands_[index].gainDB = gainDB;
        calculateCoefficients(bands_[index]);
    }
}

void ParametricEQ::setBandEnabled(int index, bool enabled) {
    if (index >= 0 && index < static_cast<int>(bands_.size())) {
        bands_[index].enabled = enabled;
    }
}

void ParametricEQ::calculateCoefficients(EQBand& band) {
    const float pi = 3.14159265358979323846f;
    float omega = 2.0f * pi * band.frequency / sampleRate_;
    float sinOmega = sin(omega);
    float cosOmega = cos(omega);
    float alpha = sinOmega / (2.0f * band.Q);
    float A = pow(10.0f, band.gainDB / 40.0f);
    
    switch (band.type) {
        case PEAK: {
            band.b0 = 1.0f + alpha * A;
            band.b1 = -2.0f * cosOmega;
            band.b2 = 1.0f - alpha * A;
            float a0 = 1.0f + alpha / A;
            band.a1 = -2.0f * cosOmega;
            band.a2 = 1.0f - alpha / A;
            
            // Normalize
            band.b0 /= a0;
            band.b1 /= a0;
            band.b2 /= a0;
            band.a1 /= a0;
            band.a2 /= a0;
            break;
        }
        case LOWSHELF: {
            float sqrtA = sqrt(A);
            band.b0 = A * ((A + 1.0f) - (A - 1.0f) * cosOmega + 2.0f * sqrtA * alpha);
            band.b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosOmega);
            band.b2 = A * ((A + 1.0f) - (A - 1.0f) * cosOmega - 2.0f * sqrtA * alpha);
            float a0 = (A + 1.0f) + (A - 1.0f) * cosOmega + 2.0f * sqrtA * alpha;
            band.a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * cosOmega);
            band.a2 = (A + 1.0f) + (A - 1.0f) * cosOmega - 2.0f * sqrtA * alpha;
            
            // Normalize
            band.b0 /= a0;
            band.b1 /= a0;
            band.b2 /= a0;
            band.a1 /= a0;
            band.a2 /= a0;
            break;
        }
        case HIGHSHELF: {
            float sqrtA = sqrt(A);
            band.b0 = A * ((A + 1.0f) + (A - 1.0f) * cosOmega + 2.0f * sqrtA * alpha);
            band.b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosOmega);
            band.b2 = A * ((A + 1.0f) + (A - 1.0f) * cosOmega - 2.0f * sqrtA * alpha);
            float a0 = (A + 1.0f) - (A - 1.0f) * cosOmega + 2.0f * sqrtA * alpha;
            band.a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cosOmega);
            band.a2 = (A + 1.0f) - (A - 1.0f) * cosOmega - 2.0f * sqrtA * alpha;
            
            // Normalize
            band.b0 /= a0;
            band.b1 /= a0;
            band.b2 /= a0;
            band.a1 /= a0;
            band.a2 /= a0;
            break;
        }
        default:
            // Default to peak
            band.b0 = 1.0f;
            band.b1 = 0.0f;
            band.b2 = 0.0f;
            band.a1 = 0.0f;
            band.a2 = 0.0f;
            break;
    }
}

float ParametricEQ::processSample(float input, EQBand& band, int channel) {
    float output = band.b0 * input + band.b1 * band.x1[channel] + band.b2 * band.x2[channel]
                   - band.a1 * band.y1[channel] - band.a2 * band.y2[channel];
    
    band.x2[channel] = band.x1[channel];
    band.x1[channel] = input;
    band.y2[channel] = band.y1[channel];
    band.y1[channel] = output;
    
    return output;
}

void ParametricEQ::process(float** inputs, float** outputs, int numChannels, int numFrames) {
    if (isBypassed()) return;
    
    for (int ch = 0; ch < numChannels; ++ch) {
        for (int i = 0; i < numFrames; ++i) {
            float sample = outputs[ch][i];
            
            // Process through each enabled band
            for (auto& band : bands_) {
                if (band.enabled) {
                    sample = processSample(sample, band, ch);
                }
            }
            
            outputs[ch][i] = sample;
        }
    }
}

// ===== Tube Saturation =====

TubeSaturation::TubeSaturation(float drive)
    : drive_(drive)
    , bias_(0.0f)
    , tone_(0.5f)
    , sampleRate_(48000) {
}

void TubeSaturation::prepare(int sampleRate, int maxBufferSize) {
    sampleRate_ = sampleRate;
    toneFilterState_.resize(2, 0.0f); // Stereo
}

void TubeSaturation::setDrive(float drive) {
    drive_ = std::max(0.0f, std::min(10.0f, drive));
}

void TubeSaturation::setBias(float bias) {
    bias_ = std::max(-0.5f, std::min(0.5f, bias));
}

void TubeSaturation::setTone(float tone) {
    tone_ = std::max(0.0f, std::min(1.0f, tone));
}

float TubeSaturation::saturate(float input) {
    // Add bias for asymmetric saturation
    input += bias_;
    
    // Tube-style saturation using tanh with overdrive
    float driven = input * drive_;
    float saturated = std::tanh(driven);
    
    // Add even-order harmonics
    saturated = saturated + 0.1f * driven * driven * (driven > 0 ? 1.0f : -1.0f);
    
    // Compensate for drive gain
    saturated /= (1.0f + drive_ * 0.3f);
    
    return saturated;
}

void TubeSaturation::process(float** inputs, float** outputs, int numChannels, int numFrames) {
    if (isBypassed()) return;
    
    // Tone filter coefficient (simple one-pole lowpass)
    float toneCoeff = 1.0f - tone_;
    
    for (int ch = 0; ch < numChannels && ch < static_cast<int>(toneFilterState_.size()); ++ch) {
        for (int i = 0; i < numFrames; ++i) {
            // Apply saturation
            float saturated = saturate(outputs[ch][i]);
            
            // Apply tone control (high frequency rolloff)
            toneFilterState_[ch] = saturated + toneCoeff * (toneFilterState_[ch] - saturated);
            
            outputs[ch][i] = toneFilterState_[ch];
        }
    }
}

// ===== Convolution Reverb =====

ConvolutionReverb::ConvolutionReverb()
    : historyPosition_(0)
    , dryWet_(0.3f)
    , preDelayFrames_(0)
    , preDelayPosition_(0)
    , sampleRate_(48000) {
}

void ConvolutionReverb::prepare(int sampleRate, int maxBufferSize) {
    sampleRate_ = sampleRate;
}

void ConvolutionReverb::setDryWetMix(float mix) {
    dryWet_ = std::max(0.0f, std::min(1.0f, mix));
}

void ConvolutionReverb::setPreDelay(float delayMs) {
    preDelayFrames_ = static_cast<int>(delayMs * sampleRate_ / 1000.0f);
    preDelayBuffer_.resize(preDelayFrames_, 0.0f);
    preDelayPosition_ = 0;
}

bool ConvolutionReverb::loadImpulseResponse(const std::string& filename) {
    // TODO: Implement WAV file loading
    // For now, create a simple synthetic impulse response
    impulseResponse_.resize(sampleRate_ / 2); // 0.5 second reverb
    
    // Simple exponential decay with noise
    for (size_t i = 0; i < impulseResponse_.size(); ++i) {
        float decay = exp(-5.0f * i / impulseResponse_.size());
        float noise = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        impulseResponse_[i] = noise * decay * 0.3f;
    }
    
    inputHistory_.resize(impulseResponse_.size(), 0.0f);
    historyPosition_ = 0;
    
    std::cout << "Loaded impulse response: " << impulseResponse_.size() << " samples" << std::endl;
    return true;
}

void ConvolutionReverb::process(float** inputs, float** outputs, int numChannels, int numFrames) {
    if (isBypassed() || impulseResponse_.empty()) return;
    
    for (int ch = 0; ch < numChannels; ++ch) {
        for (int i = 0; i < numFrames; ++i) {
            float dry = outputs[ch][i];
            
            // Store in history
            inputHistory_[historyPosition_] = dry;
            
            // Convolution (simplified - real implementation would use FFT)
            float wet = 0.0f;
            for (size_t j = 0; j < impulseResponse_.size(); ++j) {
                int historyIdx = (historyPosition_ - j + inputHistory_.size()) % inputHistory_.size();
                wet += inputHistory_[historyIdx] * impulseResponse_[j];
            }
            
            historyPosition_ = (historyPosition_ + 1) % inputHistory_.size();
            
            // Mix dry and wet
            outputs[ch][i] = dry * (1.0f - dryWet_) + wet * dryWet_;
        }
    }
}

// ===== Spectral Gate =====

SpectralGate::SpectralGate()
    : threshold_(-40.0f)
    , reduction_(-60.0f)
    , attackCoeff_(0.99f)
    , releaseCoeff_(0.999f)
    , sampleRate_(48000) {
}

void SpectralGate::prepare(int sampleRate, int maxBufferSize) {
    sampleRate_ = sampleRate;
    envelopes_.resize(2, 0.0f); // Stereo
}

void SpectralGate::setThreshold(float thresholdDB) {
    threshold_ = thresholdDB;
}

void SpectralGate::setReduction(float reductionDB) {
    reduction_ = std::min(0.0f, reductionDB);
}

void SpectralGate::setAttack(float attackMs) {
    attackCoeff_ = exp(-1.0f / (attackMs * sampleRate_ * 0.001f));
}

void SpectralGate::setRelease(float releaseMs) {
    releaseCoeff_ = exp(-1.0f / (releaseMs * sampleRate_ * 0.001f));
}

void SpectralGate::process(float** inputs, float** outputs, int numChannels, int numFrames) {
    if (isBypassed()) return;
    
    float thresholdLin = pow(10.0f, threshold_ / 20.0f);
    float reductionLin = pow(10.0f, reduction_ / 20.0f);
    
    for (int ch = 0; ch < numChannels && ch < static_cast<int>(envelopes_.size()); ++ch) {
        for (int i = 0; i < numFrames; ++i) {
            float input = std::abs(outputs[ch][i]);
            
            // Envelope follower
            float coeff = (input > envelopes_[ch]) ? attackCoeff_ : releaseCoeff_;
            envelopes_[ch] = input + coeff * (envelopes_[ch] - input);
            
            // Apply gate
            float gain = 1.0f;
            if (envelopes_[ch] < thresholdLin) {
                gain = reductionLin;
            }
            
            outputs[ch][i] *= gain;
        }
    }
}

} // namespace OmegaDAW
