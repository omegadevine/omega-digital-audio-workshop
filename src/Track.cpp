#include "Track.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace OmegaDAW {

Track::Track(const std::string& name, TrackType type)
    : name_(name)
    , type_(type)
    , trackIndex_(-1)
    , volume_(1.0f)
    , pan_(0.0f)
    , muted_(false)
    , soloed_(false)
    , recordEnabled_(false)
    , trackBuffer_(2, 512)
    , synthesizer_(nullptr) {
    
    // Create a synthesizer for MIDI tracks
    if (type_ == TrackType::MIDI) {
        synthesizer_ = std::make_shared<MIDISynthesizer>(16);
        synthesizer_->prepare(44100, 512);
        synthesizer_->setWaveform(WaveformType::Sine);
        synthesizer_->setAttack(0.01f);
        synthesizer_->setDecay(0.1f);
        synthesizer_->setSustain(0.7f);
        synthesizer_->setRelease(0.2f);
    }
}

void Track::process(AudioBuffer& buffer, int numSamples) {
    if (muted_) {
        return;
    }
    
    trackBuffer_.resize(numSamples);
    
    float leftGain = volume_;
    float rightGain = volume_;
    
    if (pan_ < 0.0f) {
        rightGain *= (1.0f + pan_);
    } else if (pan_ > 0.0f) {
        leftGain *= (1.0f - pan_);
    }
    
    for (int i = 0; i < numSamples; ++i) {
        float left = trackBuffer_.getSample(0, i) * leftGain;
        float right = trackBuffer_.getSample(1, i) * rightGain;
        
        buffer.setSample(0, i, buffer.getSample(0, i) + left);
        buffer.setSample(1, i, buffer.getSample(1, i) + right);
    }
}

void Track::setVolume(float volume) {
    volume_ = std::clamp(volume, 0.0f, 2.0f);
}

void Track::setPan(float pan) {
    pan_ = std::clamp(pan, -1.0f, 1.0f);
}

void Track::setMute(bool mute) {
    muted_ = mute;
}

void Track::setSolo(bool solo) {
    soloed_ = solo;
}

void Track::setRecordEnabled(bool enabled) {
    recordEnabled_ = enabled;
}

void Track::addClip(std::shared_ptr<Clip> clip) {
    if (!clip) return;
    clips_.push_back(clip);
    
    // Sort clips by start time
    std::sort(clips_.begin(), clips_.end(),
        [](const std::shared_ptr<Clip>& a, const std::shared_ptr<Clip>& b) {
            return a->getStartTime() < b->getStartTime();
        });
}

void Track::removeClip(size_t index) {
    if (index < clips_.size()) {
        clips_.erase(clips_.begin() + index);
    }
}

void Track::clearClips() {
    clips_.clear();
}

std::vector<std::shared_ptr<Clip>> Track::getClipsInRange(double startTime, double endTime) const {
    std::vector<std::shared_ptr<Clip>> result;
    for (const auto& clip : clips_) {
        if (clip->getEndTime() > startTime && clip->getStartTime() < endTime) {
            result.push_back(clip);
        }
    }
    return result;
}

std::shared_ptr<Clip> Track::getClipAt(double time) const {
    for (const auto& clip : clips_) {
        if (clip->isInRange(time)) {
            return clip;
        }
    }
    return nullptr;
}

void Track::processAtTime(AudioBuffer& buffer, int numSamples, double currentTime, double sampleRate) {
    if (muted_) {
        return;
    }
    
    trackBuffer_.resize(numSamples);
    trackBuffer_.clear();
    
    // Calculate time range for this buffer
    double bufferDuration = numSamples / sampleRate;
    double endTime = currentTime + bufferDuration;
    
    // Get clips that overlap with current time range
    auto activeClips = getClipsInRange(currentTime, endTime);
    
    for (const auto& clip : activeClips) {
        if (clip->getType() == ClipType::Audio) {
            auto audioClip = std::static_pointer_cast<AudioClip>(clip);
            auto audioData = audioClip->getAudioData();
            
            if (!audioData) continue;
            
            // Calculate which samples of the clip to play
            double clipStartInBuffer = clip->getStartTime() - currentTime;
            int startSample = static_cast<int>(std::max(0.0, clipStartInBuffer * sampleRate));
            
            double clipEndInBuffer = clip->getEndTime() - currentTime;
            int endSample = static_cast<int>(std::min(static_cast<double>(numSamples), clipEndInBuffer * sampleRate));
            
            // Calculate offset into clip audio data
            double offsetIntoClip = std::max(0.0, currentTime - clip->getStartTime()) + clip->getOffset();
            int audioStartSample = static_cast<int>(offsetIntoClip * sampleRate);
            
            // Copy audio data with envelope
            for (int i = startSample; i < endSample && audioStartSample < audioData->getNumSamples(); ++i, ++audioStartSample) {
                double timeInClip = clip->getStartTime() + (audioStartSample / sampleRate);
                float envelope = clip->getEnvelopeAtTime(timeInClip);
                
                for (int ch = 0; ch < std::min(trackBuffer_.getNumChannels(), audioData->getNumChannels()); ++ch) {
                    float sample = audioData->getSample(ch, audioStartSample) * envelope;
                    float current = trackBuffer_.getSample(ch, i);
                    trackBuffer_.setSample(ch, i, current + sample);
                }
            }
        } else if (clip->getType() == ClipType::MIDI && synthesizer_) {
            // Process MIDI clip through synthesizer
            auto midiClip = std::static_pointer_cast<MIDIClip>(clip);
            const auto& notes = midiClip->getNotes();
            
            // Send MIDI messages to synthesizer that fall within this buffer
            for (const auto& note : notes) {
                double noteTimeInProject = clip->getStartTime() + note.getTimestamp() - clip->getOffset();
                
                // Check if note falls within current buffer time range
                if (noteTimeInProject >= currentTime && noteTimeInProject < endTime) {
                    synthesizer_->processMIDIMessage(note);
                }
            }
            
            // Generate audio from synthesizer
            if (synthesizer_) {
                // Prepare buffer for synthesizer output
                float* outputs[2];
                std::vector<float> leftChannel(numSamples);
                std::vector<float> rightChannel(numSamples);
                outputs[0] = leftChannel.data();
                outputs[1] = rightChannel.data();
                
                // Process synthesizer
                synthesizer_->process(nullptr, outputs, 2, numSamples);
                
                // Mix synthesizer output into track buffer
                for (int i = 0; i < numSamples; ++i) {
                    float currentL = trackBuffer_.getSample(0, i);
                    float currentR = trackBuffer_.getSample(1, i);
                    trackBuffer_.setSample(0, i, currentL + outputs[0][i]);
                    trackBuffer_.setSample(1, i, currentR + outputs[1][i]);
                }
            }
        }
    }
    
    // Apply volume and pan
    float leftGain = volume_;
    float rightGain = volume_;
    
    if (pan_ < 0.0f) {
        rightGain *= (1.0f + pan_);
    } else if (pan_ > 0.0f) {
        leftGain *= (1.0f - pan_);
    }
    
    // Mix into output buffer
    for (int i = 0; i < numSamples; ++i) {
        float left = trackBuffer_.getSample(0, i) * leftGain;
        float right = trackBuffer_.getSample(1, i) * rightGain;
        
        buffer.setSample(0, i, buffer.getSample(0, i) + left);
        buffer.setSample(1, i, buffer.getSample(1, i) + right);
    }
}

} // namespace OmegaDAW
