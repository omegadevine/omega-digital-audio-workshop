#ifndef OMEGA_DAW_PROJECT_PROCESSOR_H
#define OMEGA_DAW_PROJECT_PROCESSOR_H

#include "AudioEngine.h"
#include "Project.h"
#include "Transport.h"
#include <memory>

namespace OmegaDAW {

class ProjectProcessor : public IAudioProcessor {
public:
    ProjectProcessor(std::shared_ptr<Project> project, std::shared_ptr<Transport> transport)
        : project_(project), transport_(transport), sampleRate_(44100) {}
    
    void prepare(int sampleRate, int maxBufferSize) override {
        sampleRate_ = sampleRate;
    }
    
    void process(float** inputs, float** outputs, int numChannels, int numFrames) override {
        if (isBypassed() || !project_ || !transport_) {
            // Clear outputs if bypassed
            for (int ch = 0; ch < numChannels; ++ch) {
                for (int i = 0; i < numFrames; ++i) {
                    outputs[ch][i] = 0.0f;
                }
            }
            return;
        }
        
        if (!transport_->isPlaying()) {
            // Clear outputs if not playing
            for (int ch = 0; ch < numChannels; ++ch) {
                for (int i = 0; i < numFrames; ++i) {
                    outputs[ch][i] = 0.0f;
                }
            }
            return;
        }
        
        // Get current playback position BEFORE advancing
        double currentTime = transport_->getPositionSeconds();
        
        // Create audio buffer
        AudioBuffer buffer(numChannels, numFrames);
        buffer.clear();
        
        // Process project audio at current position
        project_->processAudio(buffer, currentTime, sampleRate_);
        
        // Copy to output
        for (int ch = 0; ch < numChannels; ++ch) {
            for (int i = 0; i < numFrames; ++i) {
                outputs[ch][i] = buffer.getSample(ch, i);
            }
        }
        
        // Advance transport by the number of frames processed
        // This keeps transport in perfect sync with audio output
        transport_->advance(numFrames);
    }
    
    std::string getName() const override { return "Project Processor"; }
    
private:
    std::shared_ptr<Project> project_;
    std::shared_ptr<Transport> transport_;
    double sampleRate_;
};

} // namespace OmegaDAW

#endif // OMEGA_DAW_PROJECT_PROCESSOR_H
