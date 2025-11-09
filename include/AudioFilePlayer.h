#ifndef OMEGA_DAW_AUDIO_FILE_PLAYER_H
#define OMEGA_DAW_AUDIO_FILE_PLAYER_H

#include "AudioEngine.h"
#include "FileIO.h"
#include <vector>
#include <string>
#include <memory>
#include <atomic>
#include <mutex>

namespace OmegaDAW {

// Audio file playback processor
class AudioFilePlayer : public IAudioProcessor {
public:
    AudioFilePlayer();
    ~AudioFilePlayer() override;
    
    // Load audio file
    bool loadFile(const std::string& filepath);
    void unload();
    
    // Playback control
    void play();
    void stop();
    void pause();
    void setLoop(bool loop);
    bool isLooping() const { return looping_; }
    
    // Position control
    void setPosition(double seconds);
    void setPositionSamples(size_t samplePosition);
    double getPosition() const;
    size_t getPositionSamples() const { return playbackPosition_.load(); }
    
    // Playback state
    bool isPlaying() const { return playing_.load(); }
    bool isPaused() const { return paused_.load(); }
    bool isLoaded() const { return loaded_; }
    
    // File info
    double getDuration() const;
    size_t getTotalSamples() const { return totalSamples_; }
    int getFileSampleRate() const { return fileSampleRate_; }
    int getFileChannels() const { return fileChannels_; }
    std::string getFilePath() const { return filepath_; }
    
    // Volume control
    void setVolume(float volume);
    float getVolume() const { return volume_; }
    
    // IAudioProcessor interface
    void process(float** inputs, float** outputs, int numChannels, int numFrames) override;
    void prepare(int sampleRate, int maxBufferSize) override;
    std::string getName() const override { return "Audio File Player"; }
    
private:
    // Audio data
    std::vector<std::vector<float>> audioData_;  // Per-channel audio data
    std::string filepath_;
    
    // File properties
    int fileSampleRate_;
    int fileChannels_;
    size_t totalSamples_;
    bool loaded_;
    
    // Playback state
    std::atomic<bool> playing_;
    std::atomic<bool> paused_;
    std::atomic<size_t> playbackPosition_;
    bool looping_;
    float volume_;
    
    // Engine properties
    int engineSampleRate_;
    int engineChannels_;
    
    // Resampling
    bool needsResampling_;
    std::unique_ptr<Resampler> resampler_;
    
    // Thread safety
    mutable std::mutex dataMutex_;
    
    // Helper functions
    void reset();
    float getSample(int channel, size_t position);
};

} // namespace OmegaDAW

#endif // OMEGA_DAW_AUDIO_FILE_PLAYER_H
