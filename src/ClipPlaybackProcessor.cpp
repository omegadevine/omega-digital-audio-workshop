#include "ClipPlaybackProcessor.h"
#include <algorithm>
#include <cstring>
#include <iostream>

namespace OmegaDAW {

ClipPlaybackProcessor::ClipPlaybackProcessor(std::shared_ptr<Arrangement> arrangement,
                                             std::shared_ptr<Transport> transport,
                                             int sampleRate)
    : m_arrangement(arrangement)
    , m_transport(transport)
    , m_sampleRate(sampleRate)
    , m_invSampleRate(1.0 / sampleRate)
{
}

void ClipPlaybackProcessor::prepare(int sampleRate, int bufferSize) {
    m_sampleRate = sampleRate;
    m_invSampleRate = 1.0 / sampleRate;
    reset();
}

void ClipPlaybackProcessor::reset() {
    m_activeClips.clear();
}

void ClipPlaybackProcessor::process(float** inputs, float** outputs, int numChannels, int numFrames) {
    if (!m_arrangement || !m_transport) return;
    if (!m_transport->isPlaying()) return;
    
    // Get current transport position in samples
    double currentTime = m_transport->getPosition();
    int64_t currentSample = static_cast<int64_t>(currentTime * m_sampleRate);
    
    // Time range we're processing this buffer
    double startTime = currentTime;
    double endTime = currentTime + (numFrames * m_invSampleRate);
    
    // Process each track
    for (size_t trackIdx = 0; trackIdx < 100; ++trackIdx) {
        auto clips = m_arrangement->getClipsInTimeRange(trackIdx, startTime, endTime);
        
        for (auto& clipPtr : clips) {
            if (clipPtr->getType() != ClipType::Audio) continue;
            
            auto audioClip = std::static_pointer_cast<AudioClip>(clipPtr);
            auto audioData = audioClip->getAudioData();
            
            if (!audioData || audioData->getNumFrames() == 0) continue;
            
            // Clip timing information
            double clipStartTime = audioClip->getStartTime();
            double clipEndTime = clipStartTime + audioClip->getDuration();
            double clipOffset = audioClip->getOffset();
            
            // Calculate which samples in this buffer overlap with the clip
            int bufferStartFrame = 0;
            int bufferEndFrame = numFrames;
            
            // Clip starts after buffer start
            if (clipStartTime > startTime) {
                bufferStartFrame = static_cast<int>((clipStartTime - startTime) * m_sampleRate);
            }
            
            // Clip ends before buffer end
            if (clipEndTime < endTime) {
                bufferEndFrame = static_cast<int>((clipEndTime - startTime) * m_sampleRate);
            }
            
            // Clamp to buffer bounds
            bufferStartFrame = std::max(0, std::min(bufferStartFrame, numFrames));
            bufferEndFrame = std::max(0, std::min(bufferEndFrame, numFrames));
            
            if (bufferStartFrame >= bufferEndFrame) continue;
            
            // Calculate position in source audio
            double timeIntoClip = startTime - clipStartTime + (bufferStartFrame * m_invSampleRate);
            double sourceTime = timeIntoClip + clipOffset;
            int sourceFrame = static_cast<int>(sourceTime * m_sampleRate);
            
            // Don't play if before clip start or after end
            if (sourceFrame < 0) {
                int skipFrames = -sourceFrame;
                bufferStartFrame += skipFrames;
                sourceFrame = 0;
            }
            
            int sourceFramesAvailable = audioData->getNumFrames() - sourceFrame;
            if (sourceFramesAvailable <= 0) continue;
            
            // Limit frames to copy
            int framesToCopy = std::min(bufferEndFrame - bufferStartFrame, sourceFramesAvailable);
            if (framesToCopy <= 0) continue;
            
            // Get envelope/gain for this clip
            float gain = audioClip->getGain();
            
            // Mix audio into output
            int sourceChannels = audioData->getNumChannels();
            for (int frame = 0; frame < framesToCopy; ++frame) {
                int bufferFrame = bufferStartFrame + frame;
                int srcFrame = sourceFrame + frame;
                
                if (bufferFrame >= numFrames || srcFrame >= audioData->getNumFrames()) break;
                
                // Calculate envelope at this time
                double frameTime = startTime + (bufferFrame * m_invSampleRate);
                float envelope = audioClip->getEnvelopeAtTime(frameTime);
                float finalGain = gain * envelope;
                
                // Mix source channels to output channels
                for (int outCh = 0; outCh < numChannels; ++outCh) {
                    int srcCh = std::min(outCh, sourceChannels - 1); // Mono to stereo if needed
                    float sample = audioData->getSample(srcCh, srcFrame) * finalGain;
                    outputs[outCh][bufferFrame] += sample;
                }
            }
        }
    }
}

} // namespace OmegaDAW
