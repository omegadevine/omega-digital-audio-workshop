#include "Sequencer.h"
#include <cmath>

namespace OmegaDAW {

Sequencer::Sequencer(AudioEngine& audioEngine)
    : m_audioEngine(audioEngine)
    , m_quantization(0.25)
    , m_recording(false)
    , m_recordTrackIndex(0)
    , m_punchEnabled(false)
    , m_punchIn(0.0)
    , m_punchOut(0.0)
    , m_metronomeEnabled(false)
    , m_currentBeat(0)
    , m_lastBeatTime(0.0)
    , m_countInEnabled(false)
    , m_countInBars(1)
    , m_countInBeatsRemaining(0)
{
}

void Sequencer::setArrangement(std::shared_ptr<Arrangement> arrangement) {
    m_arrangement = arrangement;
}

void Sequencer::setTransport(std::shared_ptr<Transport> transport) {
    m_transport = transport;
}

void Sequencer::process(double deltaTime) {
    if (!m_arrangement || !m_transport) return;
    if (!m_transport->isPlaying()) return;
    
    double currentTime = m_transport->getPosition();
    
    if (m_countInEnabled && m_countInBeatsRemaining > 0) {
        processMetronome(currentTime);
        m_countInBeatsRemaining--;
        if (m_countInBeatsRemaining == 0) {
            m_transport->setPosition(0.0);
        }
        return;
    }
    
    handleLooping(currentTime);
    
    processAudioClips(currentTime, deltaTime);
    processMIDIClips(currentTime, deltaTime);
    processAutomation(currentTime);
    
    if (m_metronomeEnabled) {
        processMetronome(currentTime);
    }
    
    if (m_recording) {
        bool shouldRecord = true;
        if (m_punchEnabled) {
            shouldRecord = (currentTime >= m_punchIn && currentTime < m_punchOut);
        }
        
        if (shouldRecord) {
            // Recording logic handled by external calls to recordAudio/recordMIDI
        } else if (m_recordingClip || m_recordingMIDIClip) {
            m_recordingClip = nullptr;
            m_recordingMIDIClip = nullptr;
        }
    }
    
    if (m_playheadCallback) {
        m_playheadCallback(currentTime);
    }
}

void Sequencer::processAudioClips(double currentTime, double deltaTime) {
    for (size_t trackIdx = 0; trackIdx < 100; ++trackIdx) {
        auto clips = m_arrangement->getClipsInTimeRange(trackIdx, currentTime, currentTime + deltaTime);
        
        for (auto& clip : clips) {
            if (clip->getType() != ClipType::Audio) continue;
            
            auto audioClip = std::static_pointer_cast<AudioClip>(clip);
            auto audioData = audioClip->getAudioData();
            
            if (!audioData) continue;
            
            double clipRelativeTime = currentTime - clip->getStartTime() + clip->getOffset();
            float envelope = clip->getEnvelopeAtTime(currentTime);
            
            // Audio playback would be handled by the audio engine
            // This is where we'd schedule or trigger audio buffer playback
        }
    }
}

void Sequencer::processMIDIClips(double currentTime, double deltaTime) {
    for (size_t trackIdx = 0; trackIdx < 100; ++trackIdx) {
        auto clips = m_arrangement->getClipsInTimeRange(trackIdx, currentTime, currentTime + deltaTime);
        
        for (auto& clip : clips) {
            if (clip->getType() != ClipType::MIDI) continue;
            
            auto midiClip = std::static_pointer_cast<MIDIClip>(clip);
            double clipStartTime = clip->getStartTime();
            
            auto notes = midiClip->getNotesInRange(
                currentTime - clipStartTime,
                currentTime + deltaTime - clipStartTime
            );
            
            float envelope = clip->getEnvelopeAtTime(currentTime);
            
            for (auto& note : notes) {
                if (note.type == MIDIMessageType::NoteOn) {
                    uint8_t velocity = static_cast<uint8_t>(note.data2 * envelope);
                    MIDIMessage adjustedNote = note;
                    adjustedNote.data2 = velocity;
                    // Send MIDI note to appropriate destination
                }
            }
        }
    }
}

void Sequencer::processAutomation(double currentTime) {
    for (size_t trackIdx = 0; trackIdx < 100; ++trackIdx) {
        auto clips = m_arrangement->getClipsOnTrack(trackIdx);
        
        for (auto& clip : clips) {
            if (clip->getType() != ClipType::Automation) continue;
            if (!clip->isInRange(currentTime)) continue;
            
            auto automationClip = std::static_pointer_cast<AutomationClip>(clip);
            double clipRelativeTime = currentTime - clip->getStartTime();
            float value = automationClip->getValueAtTime(clipRelativeTime);
            
            // Apply automation value to target parameter
            // This would integrate with the mixer and plugin system
        }
    }
}

void Sequencer::processMetronome(double currentTime) {
    if (!m_transport) return;
    
    double tempo = m_transport->getTempo();
    double beatDuration = 60.0 / tempo;
    auto timeSignature = m_arrangement->getTimeSignatureAt(currentTime);
    
    int currentBeat = static_cast<int>(currentTime / beatDuration);
    
    if (currentBeat != m_currentBeat) {
        m_currentBeat = currentBeat;
        
        bool isDownbeat = (currentBeat % timeSignature.numerator) == 0;
        
        // Trigger metronome click
        // Would generate a short click sound, higher pitch for downbeat
    }
}

void Sequencer::handleLooping(double& currentTime) {
    if (!m_arrangement->isLoopEnabled()) return;
    
    double loopStart = m_arrangement->getLoopStart();
    double loopEnd = m_arrangement->getLoopEnd();
    
    if (currentTime >= loopEnd) {
        double loopLength = loopEnd - loopStart;
        if (loopLength > 0.0) {
            currentTime = loopStart + std::fmod(currentTime - loopStart, loopLength);
            m_transport->setPosition(currentTime);
        }
    }
}

void Sequencer::scheduleClipsForPlayback() {
    // Pre-schedule clips for optimized playback
    if (!m_arrangement || !m_transport) return;
    
    double currentTime = m_transport->getPosition();
    double lookAhead = 1.0;
    
    // Schedule all clips that will play in the next second
    for (size_t trackIdx = 0; trackIdx < 100; ++trackIdx) {
        auto clips = m_arrangement->getClipsInTimeRange(trackIdx, currentTime, currentTime + lookAhead);
        // Schedule these clips with the audio engine
    }
}

void Sequencer::stopAllClips() {
    // Stop all currently playing clips and MIDI notes
    m_recordingClip = nullptr;
    m_recordingMIDIClip = nullptr;
}

void Sequencer::recordAudio(const AudioBuffer& buffer) {
    if (!m_recording || !m_arrangement || !m_transport) return;
    
    bool shouldRecord = true;
    if (m_punchEnabled) {
        double currentTime = m_transport->getPosition();
        shouldRecord = (currentTime >= m_punchIn && currentTime < m_punchOut);
    }
    
    if (!shouldRecord) return;
    
    if (!m_recordingClip) {
        double startTime = m_transport->getPosition();
        if (m_arrangement->getSnapToGrid()) {
            startTime = m_arrangement->snapTimeToGrid(startTime);
        }
        
        m_recordingClip = std::make_shared<AudioClip>(startTime, 0.0);
        m_recordingClip->setName("Recorded Audio");
        // Initialize audio buffer for recording
    }
    
    // Append audio buffer to recording clip
    // Update clip duration
}

void Sequencer::recordMIDI(const MIDIMessage& message) {
    if (!m_recording || !m_arrangement || !m_transport) return;
    
    bool shouldRecord = true;
    if (m_punchEnabled) {
        double currentTime = m_transport->getPosition();
        shouldRecord = (currentTime >= m_punchIn && currentTime < m_punchOut);
    }
    
    if (!shouldRecord) return;
    
    if (!m_recordingMIDIClip) {
        double startTime = m_transport->getPosition();
        if (m_arrangement->getSnapToGrid()) {
            startTime = m_arrangement->snapTimeToGrid(startTime);
        }
        
        m_recordingMIDIClip = std::make_shared<MIDIClip>(startTime, 0.0);
        m_recordingMIDIClip->setName("Recorded MIDI");
        m_arrangement->addClip(m_recordTrackIndex, m_recordingMIDIClip);
    }
    
    double currentTime = m_transport->getPosition();
    double clipRelativeTime = currentTime - m_recordingMIDIClip->getStartTime();
    
    MIDIMessage recordedMessage = message;
    recordedMessage.timestamp = clipRelativeTime;
    
    if (m_quantization > 0.0) {
        recordedMessage.timestamp = std::round(clipRelativeTime / m_quantization) * m_quantization;
    }
    
    m_recordingMIDIClip->addNote(recordedMessage);
    m_recordingMIDIClip->setDuration(clipRelativeTime);
}

void Sequencer::setPunch(bool enabled, double punchIn, double punchOut) {
    m_punchEnabled = enabled;
    m_punchIn = punchIn;
    m_punchOut = punchOut;
}

} // namespace OmegaDAW
