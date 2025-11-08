#include "AudioEngine.h"
#include <portaudio.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <fstream>

namespace OmegaDAW {

// Resampler implementation
Resampler::Resampler() 
    : inputSampleRate_(0)
    , outputSampleRate_(0)
    , numChannels_(0)
    , ratio_(1.0) {
}

Resampler::~Resampler() {
}

void Resampler::initialize(int inputSampleRate, int outputSampleRate, int numChannels) {
    inputSampleRate_ = inputSampleRate;
    outputSampleRate_ = outputSampleRate;
    numChannels_ = numChannels;
    ratio_ = static_cast<double>(outputSampleRate) / inputSampleRate;
    leftover_.clear();
}

int Resampler::getOutputFrameCount(int inputFrames) const {
    return static_cast<int>(inputFrames * ratio_);
}

void Resampler::process(const float* input, float* output, int inputFrames, int& outputFrames) {
    // Simple linear interpolation resampling
    outputFrames = getOutputFrameCount(inputFrames);
    
    for (int ch = 0; ch < numChannels_; ++ch) {
        for (int outFrame = 0; outFrame < outputFrames; ++outFrame) {
            double inPos = outFrame / ratio_;
            int inFrame0 = static_cast<int>(inPos);
            int inFrame1 = std::min(inFrame0 + 1, inputFrames - 1);
            double frac = inPos - inFrame0;
            
            float sample0 = input[inFrame0 * numChannels_ + ch];
            float sample1 = input[inFrame1 * numChannels_ + ch];
            output[outFrame * numChannels_ + ch] = sample0 + (sample1 - sample0) * frac;
        }
    }
}

// RecordingBuffer implementation
RecordingBuffer::RecordingBuffer(int sampleRate, int numChannels)
    : sampleRate_(sampleRate)
    , numChannels_(numChannels)
    , numFrames_(0) {
}

void RecordingBuffer::clear() {
    data_.clear();
    numFrames_ = 0;
}

void RecordingBuffer::appendSamples(const float* samples, int numFrames) {
    int samplesToAdd = numFrames * numChannels_;
    data_.insert(data_.end(), samples, samples + samplesToAdd);
    numFrames_ += numFrames;
}

bool RecordingBuffer::saveToWav(const std::string& filename) {
    // Simple WAV file writer (16-bit PCM)
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return false;
    }
    
    // WAV header
    int dataSize = numFrames_ * numChannels_ * 2; // 16-bit samples
    int fileSize = 36 + dataSize;
    
    // RIFF chunk
    file.write("RIFF", 4);
    file.write(reinterpret_cast<const char*>(&fileSize), 4);
    file.write("WAVE", 4);
    
    // fmt chunk
    file.write("fmt ", 4);
    int fmtSize = 16;
    file.write(reinterpret_cast<const char*>(&fmtSize), 4);
    short audioFormat = 1; // PCM
    file.write(reinterpret_cast<const char*>(&audioFormat), 2);
    short numChannels = numChannels_;
    file.write(reinterpret_cast<const char*>(&numChannels), 2);
    file.write(reinterpret_cast<const char*>(&sampleRate_), 4);
    int byteRate = sampleRate_ * numChannels_ * 2;
    file.write(reinterpret_cast<const char*>(&byteRate), 4);
    short blockAlign = numChannels_ * 2;
    file.write(reinterpret_cast<const char*>(&blockAlign), 2);
    short bitsPerSample = 16;
    file.write(reinterpret_cast<const char*>(&bitsPerSample), 2);
    
    // data chunk
    file.write("data", 4);
    file.write(reinterpret_cast<const char*>(&dataSize), 4);
    
    // Convert float samples to 16-bit PCM
    for (float sample : data_) {
        short pcmSample = static_cast<short>(std::max(-1.0f, std::min(1.0f, sample)) * 32767.0f);
        file.write(reinterpret_cast<const char*>(&pcmSample), 2);
    }
    
    std::cout << "Saved recording to " << filename << " (" << numFrames_ << " frames)" << std::endl;
    return true;
}

AudioEngine::AudioEngine() 
    : stream_(nullptr)
    , initialized_(false)
    , selectedDeviceIndex_(-1)
    , selectedInputDeviceIndex_(-1)
    , hasInput_(false)
    , sampleRate_(48000)
    , bufferSize_(256)
    , numChannels_(2)
    , numInputChannels_(0)
    , isPlaying_(false)
    , currentTime_(0.0)
    , currentSample_(0)
    , masterVolume_(1.0f)
    , inputLatency_(0.0)
    , outputLatency_(0.0) 
    , isRecording_(false)
    , monitoringEnabled_(false)
    , inputGain_(1.0f)
    , overdubMode_(false) {
    
    // Initialize PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio initialization failed: " << Pa_GetErrorText(err) << std::endl;
    }
}

AudioEngine::~AudioEngine() {
    shutdown();
    Pa_Terminate();
}

std::vector<AudioDeviceInfo> AudioEngine::getAvailableDevices() {
    std::vector<AudioDeviceInfo> devices;
    
    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        std::cerr << "Error getting device count: " << Pa_GetErrorText(numDevices) << std::endl;
        return devices;
    }
    
    for (int i = 0; i < numDevices; ++i) {
        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
        if (deviceInfo) {
            AudioDeviceInfo info;
            info.index = i;
            info.name = deviceInfo->name;
            info.maxInputChannels = deviceInfo->maxInputChannels;
            info.maxOutputChannels = deviceInfo->maxOutputChannels;
            info.defaultSampleRate = deviceInfo->defaultSampleRate;
            devices.push_back(info);
        }
    }
    
    return devices;
}

bool AudioEngine::selectDevice(int deviceIndex) {
    if (initialized_) {
        std::cerr << "Cannot select device while engine is initialized" << std::endl;
        return false;
    }
    
    const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(deviceIndex);
    if (!deviceInfo) {
        std::cerr << "Invalid device index: " << deviceIndex << std::endl;
        return false;
    }
    
    selectedDeviceIndex_ = deviceIndex;
    std::cout << "Selected audio device: " << deviceInfo->name << std::endl;
    return true;
}

bool AudioEngine::selectInputDevice(int deviceIndex) {
    if (initialized_) {
        std::cerr << "Cannot select input device while engine is initialized" << std::endl;
        return false;
    }
    
    const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(deviceIndex);
    if (!deviceInfo) {
        std::cerr << "Invalid input device index: " << deviceIndex << std::endl;
        return false;
    }
    
    selectedInputDeviceIndex_ = deviceIndex;
    std::cout << "Selected input device: " << deviceInfo->name << std::endl;
    return true;
}

bool AudioEngine::initialize(int sampleRate, int bufferSize, int numChannels) {
    if (initialized_) {
        std::cerr << "Audio engine already initialized" << std::endl;
        return false;
    }
    
    sampleRate_ = sampleRate;
    bufferSize_ = bufferSize;
    numChannels_ = numChannels;
    
    // Initialize metering arrays
    peakLevels_.resize(numChannels_, 0.0f);
    rmsLevels_.resize(numChannels_, 0.0f);
    
    // Initialize internal buffers
    internalBuffers_.resize(numChannels_);
    for (auto& buffer : internalBuffers_) {
        buffer.resize(bufferSize_, 0.0f);
    }
    
    // Setup PortAudio stream parameters
    PaStreamParameters outputParams;
    outputParams.device = (selectedDeviceIndex_ >= 0) ? selectedDeviceIndex_ : Pa_GetDefaultOutputDevice();
    
    if (outputParams.device == paNoDevice) {
        std::cerr << "No default output device found" << std::endl;
        return false;
    }
    
    const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(outputParams.device);
    outputParams.channelCount = std::min(numChannels_, deviceInfo->maxOutputChannels);
    outputParams.sampleFormat = paFloat32;
    outputParams.suggestedLatency = deviceInfo->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;
    
    // Open stream
    PaError err = Pa_OpenStream(
        &stream_,
        nullptr,  // No input
        &outputParams,
        sampleRate_,
        bufferSize_,
        paClipOff,  // We'll handle clipping
        &AudioEngine::paCallback,
        this  // User data
    );
    
    if (err != paNoError) {
        std::cerr << "Failed to open audio stream: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    
    // Get latency information
    const PaStreamInfo* streamInfo = Pa_GetStreamInfo(stream_);
    if (streamInfo) {
        inputLatency_ = streamInfo->inputLatency;
        outputLatency_ = streamInfo->outputLatency;
    }
    
    initialized_ = true;
    
    std::cout << "Audio Engine initialized:" << std::endl;
    std::cout << "  Device: " << deviceInfo->name << std::endl;
    std::cout << "  Sample Rate: " << sampleRate_ << " Hz" << std::endl;
    std::cout << "  Buffer Size: " << bufferSize_ << " samples" << std::endl;
    std::cout << "  Channels: " << numChannels_ << std::endl;
    std::cout << "  Output Latency: " << (outputLatency_ * 1000.0) << " ms" << std::endl;
    
    return true;
}

void AudioEngine::shutdown() {
    if (!initialized_) {
        return;
    }
    
    stopPlayback();
    
    if (stream_) {
        Pa_CloseStream(stream_);
        stream_ = nullptr;
    }
    
    clearProcessors();
    initialized_ = false;
    
    std::cout << "Audio Engine shutdown" << std::endl;
}

void AudioEngine::startPlayback() {
    if (!initialized_) {
        std::cerr << "Cannot start playback: engine not initialized" << std::endl;
        return;
    }
    
    if (isPlaying_.load()) {
        return;  // Already playing
    }
    
    PaError err = Pa_StartStream(stream_);
    if (err != paNoError) {
        std::cerr << "Failed to start stream: " << Pa_GetErrorText(err) << std::endl;
        return;
    }
    
    isPlaying_.store(true);
    std::cout << "Playback started" << std::endl;
}

void AudioEngine::stopPlayback() {
    if (!isPlaying_.load()) {
        return;  // Already stopped
    }
    
    isPlaying_.store(false);
    
    if (stream_) {
        Pa_StopStream(stream_);
    }
    
    currentTime_.store(0.0);
    currentSample_.store(0);
    resetMetering();
    
    std::cout << "Playback stopped" << std::endl;
}

void AudioEngine::pausePlayback() {
    if (!isPlaying_.load()) {
        return;  // Already paused
    }
    
    isPlaying_.store(false);
    
    if (stream_) {
        Pa_StopStream(stream_);
    }
    
    std::cout << "Playback paused at " << currentTime_.load() << " seconds" << std::endl;
}

void AudioEngine::startRecording() {
    if (!hasInput_) {
        std::cerr << "Cannot start recording: no input device initialized" << std::endl;
        return;
    }
    
    if (isRecording_.load()) {
        std::cerr << "Already recording" << std::endl;
        return;
    }
    
    std::lock_guard<std::mutex> lock(recordingMutex_);
    recordingBuffer_ = std::make_shared<RecordingBuffer>(sampleRate_, numInputChannels_);
    isRecording_.store(true);
    
    std::cout << "Recording started" << std::endl;
}

void AudioEngine::stopRecording() {
    if (!isRecording_.load()) {
        return;
    }
    
    isRecording_.store(false);
    
    std::lock_guard<std::mutex> lock(recordingMutex_);
    if (recordingBuffer_) {
        std::cout << "Recording stopped (" << recordingBuffer_->getNumFrames() << " frames)" << std::endl;
    }
}

float AudioEngine::getCPULoad() const {
    if (stream_) {
        return static_cast<float>(Pa_GetStreamCpuLoad(stream_));
    }
    return 0.0f;
}

void AudioEngine::addProcessor(std::shared_ptr<IAudioProcessor> processor) {
    std::lock_guard<std::mutex> lock(processorMutex_);
    
    // Prepare the processor
    processor->prepare(sampleRate_, bufferSize_);
    processors_.push_back(processor);
    
    std::cout << "Audio processor added (total: " << processors_.size() << ")" << std::endl;
}

void AudioEngine::removeProcessor(std::shared_ptr<IAudioProcessor> processor) {
    std::lock_guard<std::mutex> lock(processorMutex_);
    
    auto it = std::find(processors_.begin(), processors_.end(), processor);
    if (it != processors_.end()) {
        processors_.erase(it);
        std::cout << "Audio processor removed (remaining: " << processors_.size() << ")" << std::endl;
    }
}

void AudioEngine::clearProcessors() {
    std::lock_guard<std::mutex> lock(processorMutex_);
    processors_.clear();
    std::cout << "All audio processors cleared" << std::endl;
}

void AudioEngine::setMasterVolume(float volume) {
    masterVolume_ = std::max(0.0f, std::min(1.0f, volume));
}

float AudioEngine::getPeakLevel(int channel) const {
    if (channel < 0 || channel >= static_cast<int>(peakLevels_.size())) {
        return 0.0f;
    }
    return peakLevels_[channel];
}

float AudioEngine::getRMSLevel(int channel) const {
    if (channel < 0 || channel >= static_cast<int>(rmsLevels_.size())) {
        return 0.0f;
    }
    return rmsLevels_[channel];
}

int AudioEngine::paCallback(const void* inputBuffer, void* outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           unsigned long statusFlags, void* userData) {
    
    AudioEngine* engine = static_cast<AudioEngine*>(userData);
    float* out = static_cast<float*>(outputBuffer);
    const float* in = static_cast<const float*>(inputBuffer);
    
    (void) timeInfo;  // Prevent unused variable warning
    (void) statusFlags;
    
    if (engine->isPlaying_.load()) {
        engine->processAudio(in, out, framesPerBuffer);
    } else {
        // Output silence when paused
        std::memset(out, 0, framesPerBuffer * engine->numChannels_ * sizeof(float));
    }
    
    return paContinue;
}

void AudioEngine::processAudio(const float* inputBuffer, float* outputBuffer, int numFrames) {
    // Process input if available
    if (hasInput_ && inputBuffer) {
        // Deinterleave input
        for (int frame = 0; frame < numFrames; ++frame) {
            for (int ch = 0; ch < numInputChannels_; ++ch) {
                float sample = inputBuffer[frame * numInputChannels_ + ch] * inputGain_;
                inputBuffers_[ch][frame] = sample;
            }
        }
        
        // Record if enabled
        if (isRecording_.load()) {
            std::lock_guard<std::mutex> lock(recordingMutex_);
            if (recordingBuffer_) {
                recordingBuffer_->appendSamples(inputBuffer, numFrames);
            }
        }
    }
    
    // Deinterleave output buffer for processing
    float** outputs = new float*[numChannels_];
    for (int ch = 0; ch < numChannels_; ++ch) {
        outputs[ch] = internalBuffers_[ch].data();
        std::memset(outputs[ch], 0, numFrames * sizeof(float));
    }
    
    // Process through audio graph
    {
        std::lock_guard<std::mutex> lock(processorMutex_);
        
        // If monitoring is enabled, pass input to processors
        float** inputs = nullptr;
        if (monitoringEnabled_ && hasInput_ && inputBuffer) {
            inputs = new float*[numInputChannels_];
            for (int ch = 0; ch < numInputChannels_; ++ch) {
                inputs[ch] = inputBuffers_[ch].data();
            }
        }
        
        for (auto& processor : processors_) {
            processor->process(inputs, outputs, numChannels_, numFrames);
        }
        
        if (inputs) {
            delete[] inputs;
        }
    }
    
    // Apply master volume and interleave output
    for (int frame = 0; frame < numFrames; ++frame) {
        for (int ch = 0; ch < numChannels_; ++ch) {
            float sample = outputs[ch][frame] * masterVolume_;
            
            // Add direct monitoring if enabled and no overdub
            if (monitoringEnabled_ && hasInput_ && !overdubMode_ && inputBuffer && ch < numInputChannels_) {
                sample += inputBuffers_[ch][frame] * masterVolume_;
            }
            
            // Soft clipping
            if (sample > 1.0f) sample = 1.0f;
            else if (sample < -1.0f) sample = -1.0f;
            
            outputBuffer[frame * numChannels_ + ch] = sample;
        }
    }
    
    delete[] outputs;
    
    // Update metering
    updateMetering(outputBuffer, numFrames);
    
    // Update time and sample count
    currentSample_.fetch_add(numFrames);
    currentTime_.store(static_cast<double>(currentSample_.load()) / sampleRate_);
}

void AudioEngine::updateMetering(const float* buffer, int numFrames) {
    std::lock_guard<std::mutex> lock(meteringMutex_);
    
    for (int ch = 0; ch < numChannels_; ++ch) {
        float peak = 0.0f;
        float sumSquares = 0.0f;
        
        for (int frame = 0; frame < numFrames; ++frame) {
            float sample = std::abs(buffer[frame * numChannels_ + ch]);
            peak = std::max(peak, sample);
            sumSquares += sample * sample;
        }
        
        peakLevels_[ch] = peak;
        rmsLevels_[ch] = std::sqrt(sumSquares / numFrames);
    }
}

void AudioEngine::resetMetering() {
    std::lock_guard<std::mutex> lock(meteringMutex_);
    std::fill(peakLevels_.begin(), peakLevels_.end(), 0.0f);
    std::fill(rmsLevels_.begin(), rmsLevels_.end(), 0.0f);
}

} // namespace OmegaDAW
