#ifndef OMEGA_DAW_PROJECT_PROCESSOR_H
#define OMEGA_DAW_PROJECT_PROCESSOR_H

#include "AudioEngine.h"
#include "Project.h"
#include "Transport.h"
#include <memory>
#include <iostream>
#include <cmath>

namespace OmegaDAW {

class ProjectProcessor : public IAudioProcessor {
public:
    ProjectProcessor(std::shared_ptr<Project> project, std::shared_ptr<Transport> transport)
        : project_(project), transport_(transport), sampleRate_(44100) {}
    
    void prepare(int sampleRate, int maxBufferSize) override {
        sampleRate_ = sampleRate;
    }
    
    void process(float** inputs, float** outputs, int numChannels, int numFrames) override {
        if (isBypassed() || !project_ || !transport_ || !transport_->isPlaying()) {
            // Clear outputs if bypassed or not playing
            for (int ch = 0; ch < numChannels; ++ch) {
                for (int i = 0; i < numFrames; ++i) {
                    outputs[ch][i] = 0.0f;
                }
            }
            return;
        }
        
        // Get current playback position
        double currentTime = transport_->getPositionSeconds();
        
        // Debug output (first few calls only)
        static int debugCount = 0;
        if (debugCount < 5) {
            std::cout << "ProjectProcessor::process called - time: " << currentTime 
                      << ", frames: " << numFrames << std::endl;
            debugCount++;
        }
        
        // Create audio buffer
        AudioBuffer buffer(numChannels, numFrames);
        buffer.clear();
        
        // Process project audio
        project_->processAudio(buffer, currentTime, sampleRate_);
        
        // Check if we got any audio
        if (debugCount < 5) {
            float maxSample = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch) {
                for (int i = 0; i < numFrames; ++i) {
                    float sample = std::abs(buffer.getSample(ch, i));
                    if (sample > maxSample) maxSample = sample;
                }
            }
            std::cout << "  Max sample amplitude: " << maxSample << std::endl;
        }
        
        // Copy to output
        for (int ch = 0; ch < numChannels; ++ch) {
            for (int i = 0; i < numFrames; ++i) {
                outputs[ch][i] = buffer.getSample(ch, i);
            }
        }
    }
    
    std::string getName() const override { return "Project Processor"; }
    
private:
    std::shared_ptr<Project> project_;
    std::shared_ptr<Transport> transport_;
    double sampleRate_;
};

} // namespace OmegaDAW

#endif // OMEGA_DAW_PROJECT_PROCESSOR_H
