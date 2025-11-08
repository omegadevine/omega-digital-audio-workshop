#ifndef OMEGA_DAW_AUDIO_BUFFER_H
#define OMEGA_DAW_AUDIO_BUFFER_H

#include <vector>
#include <cstdint>
#include <cstring>

namespace omega {

class AudioBuffer {
public:
    AudioBuffer(int numChannels = 2, int numSamples = 0);
    ~AudioBuffer() = default;

    void resize(int numSamples);
    void clear();
    
    float* getChannelData(int channel);
    const float* getChannelData(int channel) const;
    
    int getNumChannels() const { return numChannels_; }
    int getNumSamples() const { return numSamples_; }
    
    void setSample(int channel, int sample, float value);
    float getSample(int channel, int sample) const;
    
    void copyFrom(const AudioBuffer& other);
    void addFrom(const AudioBuffer& other, float gain = 1.0f);
    
    void applyGain(float gain);
    void applyGainRamp(float startGain, float endGain);

private:
    int numChannels_;
    int numSamples_;
    std::vector<std::vector<float>> channelData_;
};

} // namespace omega

#endif // OMEGA_DAW_AUDIO_BUFFER_H
