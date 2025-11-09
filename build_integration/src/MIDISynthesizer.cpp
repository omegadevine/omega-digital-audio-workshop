#include "MIDISynthesizer.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace OmegaDAW {

MIDISynthesizer::MIDISynthesizer(int maxPolyphony)
    : maxPolyphony_(maxPolyphony)
    , sampleRate_(44100)
    , waveform_(WaveformType::Sine)
    , attack_(0.01f)
    , decay_(0.1f)
    , sustain_(0.7f)
    , release_(0.3f)
    , masterVolume_(0.5f)
    , currentTime_(0.0)
    , timeIncrement_(0.0) {
    
    voices_.resize(maxPolyphony_);
}

void MIDISynthesizer::prepare(int sampleRate, int maxBufferSize) {
    sampleRate_ = sampleRate;
    timeIncrement_ = 1.0 / sampleRate_;
    
    // Reset all voices
    for (auto& voice : voices_) {
        voice.active = false;
        voice.phase = 0.0f;
    }
    
    currentTime_ = 0.0;
}

void MIDISynthesizer::process(float** inputs, float** outputs, int numChannels, int numFrames) {
    if (isBypassed()) {
        // Clear outputs if bypassed
        for (int ch = 0; ch < numChannels; ++ch) {
            std::fill(outputs[ch], outputs[ch] + numFrames, 0.0f);
        }
        return;
    }
    
    // Generate audio from active voices
    for (int frame = 0; frame < numFrames; ++frame) {
        float sample = 0.0f;
        
        // Sum all active voices
        for (auto& voice : voices_) {
            if (voice.active) {
                float envelope = calculateEnvelope(voice, currentTime_);
                
                if (envelope <= 0.0001f && voice.noteNumber == -1) {
                    // Voice has finished release, deactivate it
                    voice.active = false;
                    continue;
                }
                
                voice.envelope = envelope;
                sample += generateSample(voice) * envelope * masterVolume_;
            }
        }
        
        // Write to all output channels (mono to stereo/multi)
        for (int ch = 0; ch < numChannels; ++ch) {
            outputs[ch][frame] = sample;
        }
        
        currentTime_ += timeIncrement_;
    }
}

void MIDISynthesizer::processMIDIMessage(const MIDIMessage& message) {
    if (message.isNoteOn()) {
        noteOn(message.getNoteNumber(), message.getVelocity());
    } else if (message.isNoteOff()) {
        noteOff(message.getNoteNumber());
    } else if (message.isControlChange()) {
        // Handle CC 123 (All Notes Off)
        if (message.getControllerNumber() == 123) {
            allNotesOff();
        }
    }
}

void MIDISynthesizer::processMIDIBuffer(const MIDIBuffer& buffer) {
    for (int i = 0; i < buffer.getNumMessages(); ++i) {
        processMIDIMessage(buffer.getMessage(i));
    }
}

void MIDISynthesizer::noteOn(int noteNumber, uint8_t velocity) {
    if (velocity == 0) {
        noteOff(noteNumber);
        return;
    }
    
    Voice* voice = findFreeVoice();
    if (!voice) {
        // Steal oldest voice if no free voice available
        voice = &voices_[0];
    }
    
    voice->noteNumber = noteNumber;
    voice->velocity = velocity;
    voice->frequency = noteToFrequency(noteNumber);
    voice->amplitude = velocity / 127.0f;
    voice->phase = 0.0f;
    voice->active = true;
    voice->startTime = currentTime_;
    voice->envelope = 0.0f;
}

void MIDISynthesizer::noteOff(int noteNumber) {
    Voice* voice = findVoice(noteNumber);
    if (voice) {
        // Mark note as released (keep voice active for release phase)
        voice->noteNumber = -1;
        voice->startTime = currentTime_; // Reset start time for release phase
    }
}

void MIDISynthesizer::allNotesOff() {
    for (auto& voice : voices_) {
        if (voice.active && voice.noteNumber != -1) {
            voice.noteNumber = -1;
            voice.startTime = currentTime_;
        }
    }
}

Voice* MIDISynthesizer::findFreeVoice() {
    for (auto& voice : voices_) {
        if (!voice.active) {
            return &voice;
        }
    }
    return nullptr;
}

Voice* MIDISynthesizer::findVoice(int noteNumber) {
    for (auto& voice : voices_) {
        if (voice.active && voice.noteNumber == noteNumber) {
            return &voice;
        }
    }
    return nullptr;
}

float MIDISynthesizer::generateSample(Voice& voice) {
    float sample = 0.0f;
    float phase = voice.phase;
    
    switch (waveform_) {
        case WaveformType::Sine:
            sample = std::sin(2.0f * M_PI * phase);
            break;
            
        case WaveformType::Square:
            sample = (phase < 0.5f) ? 1.0f : -1.0f;
            break;
            
        case WaveformType::Saw:
            sample = 2.0f * phase - 1.0f;
            break;
            
        case WaveformType::Triangle:
            sample = (phase < 0.5f) ? (4.0f * phase - 1.0f) : (3.0f - 4.0f * phase);
            break;
            
        case WaveformType::Noise:
            sample = 2.0f * (static_cast<float>(rand()) / RAND_MAX) - 1.0f;
            break;
    }
    
    // Update phase
    voice.phase += voice.frequency / sampleRate_;
    if (voice.phase >= 1.0f) {
        voice.phase -= 1.0f;
    }
    
    return sample * voice.amplitude;
}

float MIDISynthesizer::calculateEnvelope(Voice& voice, double currentTime) {
    double timeSinceStart = currentTime - voice.startTime;
    
    if (voice.noteNumber == -1) {
        // Release phase
        if (timeSinceStart < release_) {
            return voice.envelope * (1.0f - timeSinceStart / release_);
        } else {
            return 0.0f;
        }
    }
    
    // Attack phase
    if (timeSinceStart < attack_) {
        return timeSinceStart / attack_;
    }
    
    // Decay phase
    timeSinceStart -= attack_;
    if (timeSinceStart < decay_) {
        return 1.0f - (1.0f - sustain_) * (timeSinceStart / decay_);
    }
    
    // Sustain phase
    return sustain_;
}

float MIDISynthesizer::noteToFrequency(int noteNumber) const {
    // A4 (MIDI note 69) = 440 Hz
    return 440.0f * std::pow(2.0f, (noteNumber - 69) / 12.0f);
}

int MIDISynthesizer::getActiveVoiceCount() const {
    int count = 0;
    for (const auto& voice : voices_) {
        if (voice.active) {
            count++;
        }
    }
    return count;
}

} // namespace OmegaDAW
