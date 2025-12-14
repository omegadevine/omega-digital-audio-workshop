#pragma once

#include "AudioEngine.h"
#include "Arrangement.h"
#include "Transport.h"
#include "Clip.h"
#include <memory>
#include <vector>

namespace OmegaDAW {

class ClipPlaybackProcessor : public IAudioProcessor {
public:
    ClipPlaybackProcessor(std::shared_ptr<Arrangement> arrangement, 
                          std::shared_ptr<Transport> transport,
                          int sampleRate);
    ~ClipPlaybackProcessor() override = default;

    void process(float** inputs, float** outputs, int numChannels, int numFrames) override;
    void prepare(int sampleRate, int bufferSize) override;
    void reset() override;

private:
    std::shared_ptr<Arrangement> m_arrangement;
    std::shared_ptr<Transport> m_transport;
    int m_sampleRate;
    double m_invSampleRate;
    
    struct ClipPlaybackState {
        std::shared_ptr<AudioClip> clip;
        int trackIndex;
        int currentSamplePosition;
    };
    
    std::vector<ClipPlaybackState> m_activeClips;
};

} // namespace OmegaDAW
