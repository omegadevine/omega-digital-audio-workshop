#ifndef OMEGA_DAW_AUDIO_ENGINE_H
#define OMEGA_DAW_AUDIO_ENGINE_H

#include <vector>
#include <memory>
#include <cstdint>

namespace omega {

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();
    
    bool initialize(int sampleRate = 44100, int bufferSize = 512);
    void shutdown();
    
    void startPlayback();
    void stopPlayback();
    void pausePlayback();
    
    bool isPlaying() const;
    double getCurrentTime() const;
    
    int getSampleRate() const { return sampleRate_; }
    int getBufferSize() const { return bufferSize_; }
    
private:
    int sampleRate_;
    int bufferSize_;
    bool isPlaying_;
    double currentTime_;
    
    void audioCallback(float* outputBuffer, int numFrames);
};

} // namespace omega

#endif // OMEGA_DAW_AUDIO_ENGINE_H
