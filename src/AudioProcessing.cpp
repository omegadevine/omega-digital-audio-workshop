#include "AudioProcessing.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>

namespace OmegaDAW {

constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 2.0f * PI;

// FFTProcessor implementation
FFTProcessor::FFTProcessor(int fftSize) : fftSize_(fftSize) {
    computeTwiddles();
}

FFTProcessor::~FFTProcessor() {
}

void FFTProcessor::computeTwiddles() {
    twiddles_.resize(fftSize_);
    for (int i = 0; i < fftSize_; ++i) {
        float angle = -TWO_PI * i / fftSize_;
        twiddles_[i] = std::complex<float>(std::cos(angle), std::sin(angle));
    }
}

void FFTProcessor::forward(const float* input, std::complex<float>* output) {
    for (int i = 0; i < fftSize_; ++i) {
        output[i] = std::complex<float>(input[i], 0.0f);
    }
    fft(output, fftSize_, false);
}

void FFTProcessor::inverse(const std::complex<float>* input, float* output) {
    std::vector<std::complex<float>> temp(input, input + fftSize_);
    fft(temp.data(), fftSize_, true);
    
    for (int i = 0; i < fftSize_; ++i) {
        output[i] = temp[i].real() / fftSize_;
    }
}

void FFTProcessor::fft(std::complex<float>* data, int n, bool inverse) {
    // Bit-reversal permutation
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        
        if (i < j) {
            std::swap(data[i], data[j]);
        }
    }
    
    // Cooley-Tukey FFT
    for (int len = 2; len <= n; len <<= 1) {
        float angle = (inverse ? TWO_PI : -TWO_PI) / len;
        std::complex<float> wlen(std::cos(angle), std::sin(angle));
        
        for (int i = 0; i < n; i += len) {
            std::complex<float> w(1.0f, 0.0f);
            for (int j = 0; j < len / 2; ++j) {
                std::complex<float> u = data[i + j];
                std::complex<float> v = data[i + j + len / 2] * w;
                data[i + j] = u + v;
                data[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}

// PhaseVocoder implementation
PhaseVocoder::PhaseVocoder() 
    : sampleRate_(48000)
    , fftSize_(2048)
    , hopSize_(512)
    , timeStretchFactor_(1.0f)
    , pitchShift_(0.0f)
    , formantPreservation_(false)
    , inputPos_(0)
    , outputPos_(0) {
}

PhaseVocoder::~PhaseVocoder() {
}

void PhaseVocoder::prepare(int sampleRate, int maxBufferSize) {
    sampleRate_ = sampleRate;
    fft_ = std::make_unique<FFTProcessor>(fftSize_);
    
    inputBuffer_.resize(2);  // Stereo
    outputBuffer_.resize(2);
    for (int i = 0; i < 2; ++i) {
        inputBuffer_[i].resize(fftSize_ * 4, 0.0f);
        outputBuffer_[i].resize(fftSize_ * 4, 0.0f);
    }
    
    frequencyData_.resize(fftSize_);
    lastPhase_.resize(fftSize_, 0.0f);
    phaseSum_.resize(fftSize_, 0.0f);
}

void PhaseVocoder::setPitchShift(float semitones) {
    pitchShift_ = semitones;
}

void PhaseVocoder::process(float** inputs, float** outputs, int numChannels, int numFrames) {
    if (!fft_) return;
    
    // Simple passthrough for now - full implementation would be complex
    if (inputs) {
        for (int ch = 0; ch < numChannels; ++ch) {
            std::memcpy(outputs[ch], inputs[ch], numFrames * sizeof(float));
        }
    }
}

// SpectralProcessor implementation
SpectralProcessor::SpectralProcessor() 
    : sampleRate_(48000)
    , fftSize_(2048)
    , hopSize_(512)
    , processMode_(SPECTRAL_GATE)
    , threshold_(-40.0f)
    , amount_(1.0f)
    , bufferPos_(0)
    , spectrumFrozen_(false) {
}

SpectralProcessor::~SpectralProcessor() {
}

void SpectralProcessor::prepare(int sampleRate, int maxBufferSize) {
    sampleRate_ = sampleRate;
    fft_ = std::make_unique<FFTProcessor>(fftSize_);
    
    inputBuffer_.resize(2);
    outputBuffer_.resize(2);
    for (int i = 0; i < 2; ++i) {
        inputBuffer_[i].resize(fftSize_ * 2, 0.0f);
        outputBuffer_[i].resize(fftSize_ * 2, 0.0f);
    }
    
    frequencyData_.resize(fftSize_);
    frozenSpectrum_.resize(fftSize_);
}

void SpectralProcessor::process(float** inputs, float** outputs, int numChannels, int numFrames) {
    if (!fft_ || !inputs) {
        return;
    }
    
    // Simple passthrough for now
    for (int ch = 0; ch < numChannels; ++ch) {
        std::memcpy(outputs[ch], inputs[ch], numFrames * sizeof(float));
    }
}

void SpectralProcessor::processSpectrum(std::complex<float>* spectrum, int numBins) {
    switch (processMode_) {
        case SPECTRAL_GATE: {
            for (int i = 0; i < numBins; ++i) {
                float magnitude = std::abs(spectrum[i]);
                float magnitudeDB = 20.0f * std::log10(magnitude + 1e-10f);
                if (magnitudeDB < threshold_) {
                    spectrum[i] *= (1.0f - amount_);
                }
            }
            break;
        }
        
        case SPECTRAL_FREEZE: {
            if (spectrumFrozen_) {
                for (int i = 0; i < numBins; ++i) {
                    spectrum[i] = frozenSpectrum_[i];
                }
            } else {
                for (int i = 0; i < numBins; ++i) {
                    frozenSpectrum_[i] = spectrum[i];
                }
            }
            break;
        }
        
        default:
            break;
    }
}

// AudioAnalyzer implementation
AudioAnalyzer::AudioAnalyzer(int sampleRate, int fftSize) 
    : sampleRate_(sampleRate)
    , fftSize_(fftSize)
    , bufferPos_(0)
    , rmsLevel_(0.0f) {
    
    fft_ = std::make_unique<FFTProcessor>(fftSize_);
    inputBuffer_.resize(fftSize_, 0.0f);
    frequencyData_.resize(fftSize_);
    spectrum_.resize(fftSize_ / 2);
    window_.resize(fftSize_);
    
    computeWindow();
}

AudioAnalyzer::~AudioAnalyzer() {
}

void AudioAnalyzer::computeWindow() {
    // Hann window
    for (int i = 0; i < fftSize_; ++i) {
        window_[i] = 0.5f * (1.0f - std::cos(TWO_PI * i / (fftSize_ - 1)));
    }
}

void AudioAnalyzer::analyze(const float* input, int numFrames) {
    // Calculate RMS
    float sumSquares = 0.0f;
    for (int i = 0; i < numFrames; ++i) {
        sumSquares += input[i] * input[i];
    }
    rmsLevel_ = std::sqrt(sumSquares / numFrames);
    
    // Copy to buffer with windowing
    int framesToCopy = std::min(numFrames, fftSize_ - bufferPos_);
    for (int i = 0; i < framesToCopy; ++i) {
        inputBuffer_[bufferPos_ + i] = input[i] * window_[bufferPos_ + i];
    }
    bufferPos_ += framesToCopy;
    
    // Perform FFT when buffer is full
    if (bufferPos_ >= fftSize_) {
        fft_->forward(inputBuffer_.data(), frequencyData_.data());
        
        // Compute magnitude spectrum
        for (int i = 0; i < fftSize_ / 2; ++i) {
            float magnitude = std::abs(frequencyData_[i]);
            spectrum_[i] = 20.0f * std::log10(magnitude + 1e-10f);
        }
        
        // Reset buffer
        bufferPos_ = 0;
        std::fill(inputBuffer_.begin(), inputBuffer_.end(), 0.0f);
    }
}

float AudioAnalyzer::getMagnitudeAtFrequency(float frequency) const {
    int bin = static_cast<int>(frequency * fftSize_ / sampleRate_);
    bin = std::max(0, std::min(bin, (int)spectrum_.size() - 1));
    return spectrum_[bin];
}

float AudioAnalyzer::getPeakFrequency() const {
    auto maxIt = std::max_element(spectrum_.begin(), spectrum_.end());
    int peakBin = std::distance(spectrum_.begin(), maxIt);
    return peakBin * sampleRate_ / static_cast<float>(fftSize_);
}

float AudioAnalyzer::getSpectralCentroid() const {
    float weightedSum = 0.0f;
    float totalMagnitude = 0.0f;
    
    for (int i = 0; i < static_cast<int>(spectrum_.size()); ++i) {
        float magnitude = std::pow(10.0f, spectrum_[i] / 20.0f);
        float frequency = i * sampleRate_ / static_cast<float>(fftSize_);
        weightedSum += frequency * magnitude;
        totalMagnitude += magnitude;
    }
    
    return (totalMagnitude > 0.0f) ? (weightedSum / totalMagnitude) : 0.0f;
}

} // namespace OmegaDAW
