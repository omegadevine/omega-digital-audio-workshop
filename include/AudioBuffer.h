#ifndef OMEGA_DAW_AUDIO_BUFFER_H
#define OMEGA_DAW_AUDIO_BUFFER_H

#include <vector>
#include <cstdint>
#include <cstring>

namespace OmegaDAW {

class AudioBuffer {
public:
    AudioBuffer(int numChannels = 2, int numSamples = 0);
    ~AudioBuffer() = default;

    void resize(int numSamples);
    void clear();
    
    float* getChannelData(int channel);
    const float* getChannelData(int channel) const;
    float* getWritePointer(int channel) { return getChannelData(channel); }
    const float* getReadPointer(int channel) const { return getChannelData(channel); }
    
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

} // namespace OmegaDAW

#endif // OMEGA_DAW_AUDIO_BUFFER_H
