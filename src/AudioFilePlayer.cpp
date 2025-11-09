#include "AudioFilePlayer.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace OmegaDAW {

AudioFilePlayer::AudioFilePlayer()
    : fileSampleRate_(0)
    , fileChannels_(0)
    , totalSamples_(0)
    , loaded_(false)
    , playing_(false)
    , paused_(false)
    , playbackPosition_(0)
    , looping_(false)
    , volume_(1.0f)
    , engineSampleRate_(48000)
    , engineChannels_(2)
    , needsResampling_(false) {
}

AudioFilePlayer::~AudioFilePlayer() {
    unload();
}

bool AudioFilePlayer::loadFile(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(dataMutex_);
    
    // Unload any existing file
    reset();
    
    // Use FileManager to import audio data
    FileManager& fileManager = FileManager::getInstance();
    FileIOResult result = fileManager.importAudioFile(filepath, audioData_, fileSampleRate_);
    
    if (!result.success) {
        std::cerr << "Failed to load audio file: " << result.errorMessage << std::endl;
        return false;
    }
    
    if (audioData_.empty() || audioData_[0].empty()) {
        std::cerr << "Audio file is empty" << std::endl;
        return false;
    }
    
    filepath_ = filepath;
    fileChannels_ = static_cast<int>(audioData_.size());
    totalSamples_ = audioData_[0].size();
    loaded_ = true;
    
    std::cout << "Loaded audio file: " << filepath << std::endl;
    std::cout << "  Channels: " << fileChannels_ << std::endl;
    std::cout << "  Sample Rate: " << fileSampleRate_ << " Hz" << std::endl;
    std::cout << "  Duration: " << getDuration() << " seconds" << std::endl;
    std::cout << "  Total Samples: " << totalSamples_ << std::endl;
    
    // Check if resampling is needed
    if (engineSampleRate_ != 0 && fileSampleRate_ != engineSampleRate_) {
        needsResampling_ = true;
        resampler_ = std::make_unique<Resampler>();
        resampler_->initialize(fileSampleRate_, engineSampleRate_, fileChannels_);
        std::cout << "  Resampling: " << fileSampleRate_ << " Hz -> " << engineSampleRate_ << " Hz" << std::endl;
    }
    
    return true;
}

void AudioFilePlayer::unload() {
    std::lock_guard<std::mutex> lock(dataMutex_);
    stop();
    reset();
}

void AudioFilePlayer::reset() {
    audioData_.clear();
    filepath_.clear();
    fileSampleRate_ = 0;
    fileChannels_ = 0;
    totalSamples_ = 0;
    loaded_ = false;
    playing_.store(false);
    paused_.store(false);
    playbackPosition_.store(0);
    needsResampling_ = false;
    resampler_.reset();
}

void AudioFilePlayer::play() {
    if (!loaded_) {
        std::cerr << "Cannot play: no file loaded" << std::endl;
        return;
    }
    
    playing_.store(true);
    paused_.store(false);
    std::cout << "Playing: " << filepath_ << std::endl;
}

void AudioFilePlayer::stop() {
    playing_.store(false);
    paused_.store(false);
    playbackPosition_.store(0);
    std::cout << "Stopped playback" << std::endl;
}

void AudioFilePlayer::pause() {
    if (playing_.load()) {
        paused_.store(true);
        playing_.store(false);
        std::cout << "Paused at " << getPosition() << " seconds" << std::endl;
    }
}

void AudioFilePlayer::setLoop(bool loop) {
    looping_ = loop;
    std::cout << "Loop " << (loop ? "enabled" : "disabled") << std::endl;
}

void AudioFilePlayer::setPosition(double seconds) {
    if (!loaded_) return;
    
    size_t samplePos = static_cast<size_t>(seconds * fileSampleRate_);
    setPositionSamples(samplePos);
}

void AudioFilePlayer::setPositionSamples(size_t samplePosition) {
    if (!loaded_) return;
    
    size_t clampedPos = std::min(samplePosition, totalSamples_);
    playbackPosition_.store(clampedPos);
}

double AudioFilePlayer::getPosition() const {
    if (!loaded_ || fileSampleRate_ == 0) return 0.0;
    return static_cast<double>(playbackPosition_.load()) / fileSampleRate_;
}

double AudioFilePlayer::getDuration() const {
    if (!loaded_ || fileSampleRate_ == 0) return 0.0;
    return static_cast<double>(totalSamples_) / fileSampleRate_;
}

void AudioFilePlayer::setVolume(float volume) {
    volume_ = std::max(0.0f, std::min(1.0f, volume));
}

float AudioFilePlayer::getSample(int channel, size_t position) {
    if (!loaded_ || position >= totalSamples_) {
        return 0.0f;
    }
    
    // Handle channel mapping
    int sourceChannel = std::min(channel, fileChannels_ - 1);
    return audioData_[sourceChannel][position];
}

void AudioFilePlayer::prepare(int sampleRate, int maxBufferSize) {
    engineSampleRate_ = sampleRate;
    
    // Update resampling if needed
    std::lock_guard<std::mutex> lock(dataMutex_);
    if (loaded_ && fileSampleRate_ != engineSampleRate_) {
        needsResampling_ = true;
        resampler_ = std::make_unique<Resampler>();
        resampler_->initialize(fileSampleRate_, engineSampleRate_, fileChannels_);
        std::cout << "Resampler prepared: " << fileSampleRate_ << " Hz -> " << engineSampleRate_ << " Hz" << std::endl;
    }
}

void AudioFilePlayer::process(float** inputs, float** outputs, int numChannels, int numFrames) {
    (void)inputs;  // Not used
    
    if (!playing_.load() || !loaded_ || bypassed_) {
        // Output silence
        for (int ch = 0; ch < numChannels; ++ch) {
            std::fill(outputs[ch], outputs[ch] + numFrames, 0.0f);
        }
        return;
    }
    
    std::lock_guard<std::mutex> lock(dataMutex_);
    
    size_t currentPos = playbackPosition_.load();
    
    if (needsResampling_ && resampler_) {
        // TODO: Implement proper resampling in process loop
        // For now, use simple nearest-neighbor sampling
        for (int frame = 0; frame < numFrames; ++frame) {
            if (currentPos >= totalSamples_) {
                if (looping_) {
                    currentPos = 0;
                } else {
                    // Fill remaining with silence
                    for (int ch = 0; ch < numChannels; ++ch) {
                        for (int f = frame; f < numFrames; ++f) {
                            outputs[ch][f] = 0.0f;
                        }
                    }
                    playing_.store(false);
                    playbackPosition_.store(0);
                    return;
                }
            }
            
            // Convert engine sample position to file sample position
            double filePos = static_cast<double>(currentPos);
            size_t fileSample = static_cast<size_t>(filePos);
            
            // Read and output samples
            for (int ch = 0; ch < numChannels; ++ch) {
                float sample = getSample(ch, fileSample) * volume_;
                outputs[ch][frame] = sample;
            }
            
            // Advance playback position (accounting for sample rate difference)
            double advance = static_cast<double>(fileSampleRate_) / engineSampleRate_;
            currentPos = static_cast<size_t>(filePos + advance);
        }
    } else {
        // No resampling needed - direct playback
        for (int frame = 0; frame < numFrames; ++frame) {
            if (currentPos >= totalSamples_) {
                if (looping_) {
                    currentPos = 0;
                } else {
                    // Fill remaining with silence
                    for (int ch = 0; ch < numChannels; ++ch) {
                        for (int f = frame; f < numFrames; ++f) {
                            outputs[ch][f] = 0.0f;
                        }
                    }
                    playing_.store(false);
                    playbackPosition_.store(0);
                    return;
                }
            }
            
            // Read and output samples
            for (int ch = 0; ch < numChannels; ++ch) {
                float sample = getSample(ch, currentPos) * volume_;
                outputs[ch][frame] = sample;
            }
            
            currentPos++;
        }
    }
    
    playbackPosition_.store(currentPos);
}

} // namespace OmegaDAW
