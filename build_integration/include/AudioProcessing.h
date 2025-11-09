#ifndef OMEGA_DAW_AUDIO_PROCESSING_H
#define OMEGA_DAW_AUDIO_PROCESSING_H

#include "AudioEngine.h"
#include <vector>
#include <complex>
#include <cmath>

namespace OmegaDAW {

// FFT-based audio processing utilities
class FFTProcessor {
public:
    FFTProcessor(int fftSize);
    ~FFTProcessor();
    
    void forward(const float* input, std::complex<float>* output);
    void inverse(const std::complex<float>* input, float* output);
    
    int getFFTSize() const { return fftSize_; }
    
private:
    int fftSize_;
    std::vector<std::complex<float>> twiddles_;
    void computeTwiddles();
    void fft(std::complex<float>* data, int n, bool inverse);
};

// Phase vocoder for time-stretching and pitch-shifting
class PhaseVocoder : public IAudioProcessor {
public:
    PhaseVocoder();
    ~PhaseVocoder();
    
    void prepare(int sampleRate, int maxBufferSize) override;
    void process(float** inputs, float** outputs, int numChannels, int numFrames) override;
    
    // Time stretch factor (1.0 = normal, 2.0 = twice as slow, 0.5 = twice as fast)
    void setTimeStretchFactor(float factor) { timeStretchFactor_ = factor; }
    float getTimeStretchFactor() const { return timeStretchFactor_; }
    
    // Pitch shift in semitones (-12 to +12)
    void setPitchShift(float semitones);
    float getPitchShift() const { return pitchShift_; }
    
    // Formant preservation
    void setFormantPreservation(bool enabled) { formantPreservation_ = enabled; }
    bool isFormantPreservationEnabled() const { return formantPreservation_; }
    
private:
    int sampleRate_;
    int fftSize_;
    int hopSize_;
    float timeStretchFactor_;
    float pitchShift_;
    bool formantPreservation_;
    
    std::unique_ptr<FFTProcessor> fft_;
    std::vector<std::vector<float>> inputBuffer_;
    std::vector<std::vector<float>> outputBuffer_;
    std::vector<std::complex<float>> frequencyData_;
    std::vector<float> lastPhase_;
    std::vector<float> phaseSum_;
    
    int inputPos_;
    int outputPos_;
};

// Convolution reverb
class ConvolutionReverb : public IAudioProcessor {
public:
    ConvolutionReverb();
    ~ConvolutionReverb();
    
    void prepare(int sampleRate, int maxBufferSize) override;
    void process(float** inputs, float** outputs, int numChannels, int numFrames) override;
    
    // Load impulse response from file or buffer
    bool loadImpulseResponse(const std::string& filename);
    void setImpulseResponse(const float* ir, int irLength, int numChannels);
    
    void setWetDryMix(float mix) { wetDryMix_ = std::max(0.0f, std::min(1.0f, mix)); }
    float getWetDryMix() const { return wetDryMix_; }
    
private:
    int sampleRate_;
    int maxBufferSize_;
    
    std::vector<std::vector<float>> impulseResponse_;
    std::vector<std::vector<float>> inputHistory_;
    int historyPos_;
    float wetDryMix_;
    
    void convolve(const float* input, float* output, int numFrames, int channel);
};

// Spectral processor for advanced effects
class SpectralProcessor : public IAudioProcessor {
public:
    enum ProcessMode {
        SPECTRAL_GATE,      // Remove frequencies below threshold
        SPECTRAL_COMPRESSOR, // Compress individual frequency bands
        SPECTRAL_FREEZE,    // Freeze spectrum
        HARMONIC_ENHANCER   // Enhance harmonics
    };
    
    SpectralProcessor();
    ~SpectralProcessor();
    
    void prepare(int sampleRate, int maxBufferSize) override;
    void process(float** inputs, float** outputs, int numChannels, int numFrames) override;
    
    void setProcessMode(ProcessMode mode) { processMode_ = mode; }
    ProcessMode getProcessMode() const { return processMode_; }
    
    void setThreshold(float threshold) { threshold_ = threshold; }
    float getThreshold() const { return threshold_; }
    
    void setAmount(float amount) { amount_ = std::max(0.0f, std::min(1.0f, amount)); }
    float getAmount() const { return amount_; }
    
private:
    int sampleRate_;
    int fftSize_;
    int hopSize_;
    ProcessMode processMode_;
    float threshold_;
    float amount_;
    
    std::unique_ptr<FFTProcessor> fft_;
    std::vector<std::vector<float>> inputBuffer_;
    std::vector<std::vector<float>> outputBuffer_;
    std::vector<std::complex<float>> frequencyData_;
    std::vector<std::complex<float>> frozenSpectrum_;
    
    int bufferPos_;
    bool spectrumFrozen_;
    
    void processSpectrum(std::complex<float>* spectrum, int numBins);
};

// Real-time audio analyzer
class AudioAnalyzer {
public:
    AudioAnalyzer(int sampleRate, int fftSize);
    ~AudioAnalyzer();
    
    void analyze(const float* input, int numFrames);
    
    // Get frequency spectrum (magnitude in dB)
    const std::vector<float>& getSpectrum() const { return spectrum_; }
    
    // Get specific frequency magnitude
    float getMagnitudeAtFrequency(float frequency) const;
    
    // Get peak frequency
    float getPeakFrequency() const;
    
    // Get spectral centroid (brightness)
    float getSpectralCentroid() const;
    
    // Get RMS level
    float getRMSLevel() const { return rmsLevel_; }
    
private:
    int sampleRate_;
    int fftSize_;
    
    std::unique_ptr<FFTProcessor> fft_;
    std::vector<float> inputBuffer_;
    std::vector<std::complex<float>> frequencyData_;
    std::vector<float> spectrum_;
    std::vector<float> window_;
    
    int bufferPos_;
    float rmsLevel_;
    
    void computeWindow();
};

} // namespace OmegaDAW

#endif // OMEGA_DAW_AUDIO_PROCESSING_H
