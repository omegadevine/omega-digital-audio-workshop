#ifndef OMEGA_DAW_AUDIO_ENGINE_H
#define OMEGA_DAW_AUDIO_ENGINE_H

#include <vector>
#include <memory>
#include <cstdint>
#include <functional>
#include <mutex>
#include <atomic>

// Forward declare PortAudio stream
typedef void PaStream;

namespace OmegaDAW {

// Audio processor interface
class IAudioProcessor {
public:
    virtual ~IAudioProcessor() = default;
    virtual void process(float** inputs, float** outputs, int numChannels, int numFrames) = 0;
    virtual void prepare(int sampleRate, int maxBufferSize) = 0;
};

// Audio device info
struct AudioDeviceInfo {
    int index;
    std::string name;
    int maxInputChannels;
    int maxOutputChannels;
    double defaultSampleRate;
};

// Resampler for handling different sample rates
class Resampler {
public:
    Resampler();
    ~Resampler();
    void initialize(int inputSampleRate, int outputSampleRate, int numChannels);
    void process(const float* input, float* output, int inputFrames, int& outputFrames);
    int getOutputFrameCount(int inputFrames) const;
private:
    int inputSampleRate_;
    int outputSampleRate_;
    int numChannels_;
    double ratio_;
    std::vector<float> leftover_;
};

// Recording buffer
class RecordingBuffer {
public:
    RecordingBuffer(int sampleRate, int numChannels);
    void clear();
    void appendSamples(const float* samples, int numFrames);
    const std::vector<float>& getData() const { return data_; }
    int getNumFrames() const { return numFrames_; }
    bool saveToWav(const std::string& filename);
private:
    int sampleRate_;
    int numChannels_;
    int numFrames_;
    std::vector<float> data_;
};

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();
    
    // Device management
    std::vector<AudioDeviceInfo> getAvailableDevices();
    bool selectDevice(int deviceIndex);
    bool selectInputDevice(int deviceIndex);
    
    // Initialization
    bool initialize(int sampleRate = 48000, int bufferSize = 256, int numChannels = 2);
    bool initializeWithInput(int sampleRate = 48000, int bufferSize = 256, 
                            int numOutputChannels = 2, int numInputChannels = 2);
    void shutdown();
    
    // Transport control
    void startPlayback();
    void stopPlayback();
    void pausePlayback();
    
    // Recording control
    void startRecording();
    void stopRecording();
    bool isRecording() const { return isRecording_.load(); }
    std::shared_ptr<RecordingBuffer> getRecordingBuffer() const { return recordingBuffer_; }
    
    // State queries
    bool isPlaying() const { return isPlaying_.load(); }
    bool isInitialized() const { return initialized_; }
    double getCurrentTime() const { return currentTime_.load(); }
    uint64_t getCurrentSample() const { return currentSample_.load(); }
    
    // Audio properties
    int getSampleRate() const { return sampleRate_; }
    int getBufferSize() const { return bufferSize_; }
    int getNumChannels() const { return numChannels_; }
    float getCPULoad() const;
    
    // Audio graph management
    void addProcessor(std::shared_ptr<IAudioProcessor> processor);
    void removeProcessor(std::shared_ptr<IAudioProcessor> processor);
    void clearProcessors();
    
    // Master volume and metering
    void setMasterVolume(float volume);
    float getMasterVolume() const { return masterVolume_; }
    float getPeakLevel(int channel) const;
    float getRMSLevel(int channel) const;
    
    // Latency
    double getInputLatency() const { return inputLatency_; }
    double getOutputLatency() const { return outputLatency_; }
    
    // Advanced features
    void setMonitoringEnabled(bool enabled) { monitoringEnabled_ = enabled; }
    bool isMonitoringEnabled() const { return monitoringEnabled_; }
    void setInputGain(float gain) { inputGain_ = std::max(0.0f, std::min(10.0f, gain)); }
    float getInputGain() const { return inputGain_; }
    
    // Overdub mode - mix recording with playback
    void setOverdubMode(bool enabled) { overdubMode_ = enabled; }
    bool isOverdubMode() const { return overdubMode_; }
    
private:
    // Audio callback (static for C API)
    static int paCallback(const void* inputBuffer, void* outputBuffer,
                         unsigned long framesPerBuffer,
                         const struct PaStreamCallbackTimeInfo* timeInfo,
                         unsigned long statusFlags, void* userData);
    
    // Instance audio processing
    void processAudio(const float* inputBuffer, float* outputBuffer, int numFrames);
    
    // Audio metering
    void updateMetering(const float* buffer, int numFrames);
    void resetMetering();
    
    // PortAudio stream
    PaStream* stream_;
    bool initialized_;
    int selectedDeviceIndex_;
    int selectedInputDeviceIndex_;
    bool hasInput_;
    
    // Audio configuration
    int sampleRate_;
    int bufferSize_;
    int numChannels_;
    int numInputChannels_;
    
    // Transport state
    std::atomic<bool> isPlaying_;
    std::atomic<double> currentTime_;
    std::atomic<uint64_t> currentSample_;
    
    // Master controls
    float masterVolume_;
    
    // Audio processors
    std::vector<std::shared_ptr<IAudioProcessor>> processors_;
    std::mutex processorMutex_;
    
    // Metering
    std::vector<float> peakLevels_;
    std::vector<float> rmsLevels_;
    std::mutex meteringMutex_;
    
    // Latency info
    double inputLatency_;
    double outputLatency_;
    
    // Internal buffers for processing
    std::vector<std::vector<float>> internalBuffers_;
    std::vector<std::vector<float>> inputBuffers_;
    
    // Recording
    std::atomic<bool> isRecording_;
    std::shared_ptr<RecordingBuffer> recordingBuffer_;
    std::mutex recordingMutex_;
    bool monitoringEnabled_;
    float inputGain_;
    bool overdubMode_;
};

} // namespace OmegaDAW

#endif // OMEGA_DAW_AUDIO_ENGINE_H
