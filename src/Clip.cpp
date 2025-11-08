#include "Clip.h"
#include <algorithm>
#include <cmath>

namespace OmegaDAW {

Clip::Clip(ClipType type, double startTime, double duration)
    : m_type(type)
    , m_startTime(startTime)
    , m_duration(duration)
    , m_offset(0.0)
    , m_loop(false)
    , m_gain(1.0f)
    , m_fadeInDuration(0.0)
    , m_fadeOutDuration(0.0)
    , m_name("Clip")
    , m_color(0xFFFFFFFF)
{
}

bool Clip::isInRange(double time) const {
    return time >= m_startTime && time < getEndTime();
}

float Clip::getEnvelopeAtTime(double time) const {
    if (!isInRange(time)) return 0.0f;
    
    double relativeTime = time - m_startTime;
    float envelope = m_gain;
    
    if (m_fadeInDuration > 0.0 && relativeTime < m_fadeInDuration) {
        envelope *= static_cast<float>(relativeTime / m_fadeInDuration);
    }
    
    double timeFromEnd = m_duration - relativeTime;
    if (m_fadeOutDuration > 0.0 && timeFromEnd < m_fadeOutDuration) {
        envelope *= static_cast<float>(timeFromEnd / m_fadeOutDuration);
    }
    
    return envelope;
}

AudioClip::AudioClip(double startTime, double duration)
    : Clip(ClipType::Audio, startTime, duration)
    , m_pitchShift(0.0f)
    , m_reverse(false)
{
}

void AudioClip::setAudioData(std::shared_ptr<AudioBuffer> buffer) {
    m_audioData = buffer;
}

MIDIClip::MIDIClip(double startTime, double duration)
    : Clip(ClipType::MIDI, startTime, duration)
{
}

void MIDIClip::addNote(const MIDIMessage& note) {
    m_notes.push_back(note);
    std::sort(m_notes.begin(), m_notes.end(), 
        [](const MIDIMessage& a, const MIDIMessage& b) {
            return a.getTimestamp() < b.getTimestamp();
        });
}

void MIDIClip::removeNote(size_t index) {
    if (index < m_notes.size()) {
        m_notes.erase(m_notes.begin() + index);
    }
}

void MIDIClip::clearNotes() {
    m_notes.clear();
}

std::vector<MIDIMessage> MIDIClip::getNotesInRange(double startTime, double endTime) const {
    std::vector<MIDIMessage> result;
    for (const auto& note : m_notes) {
        if (note.getTimestamp() >= startTime && note.getTimestamp() < endTime) {
            result.push_back(note);
        }
    }
    return result;
}

void MIDIClip::quantize(double gridSize) {
    for (auto& note : m_notes) {
        double timestamp = note.getTimestamp();
        double quantized = std::round(timestamp / gridSize) * gridSize;
        note.setTimestamp(quantized);
    }
    std::sort(m_notes.begin(), m_notes.end(), 
        [](const MIDIMessage& a, const MIDIMessage& b) {
            return a.getTimestamp() < b.getTimestamp();
        });
}

void MIDIClip::transpose(int semitones) {
    std::vector<MIDIMessage> transposedNotes;
    for (const auto& note : m_notes) {
        if (note.isNoteOn() || note.isNoteOff()) {
            int newNote = note.getNoteNumber() + semitones;
            if (newNote >= 0 && newNote <= 127) {
                MIDIMessage transposed(
                    note.getStatus(),
                    static_cast<uint8_t>(newNote),
                    note.getData2()
                );
                transposed.setTimestamp(note.getTimestamp());
                transposedNotes.push_back(transposed);
            } else {
                transposedNotes.push_back(note);
            }
        } else {
            transposedNotes.push_back(note);
        }
    }
    m_notes = transposedNotes;
}

void MIDIClip::setVelocity(uint8_t velocity) {
    for (auto& note : m_notes) {
        if (note.type == MIDIMessageType::NoteOn) {
            note.data2 = velocity;
        }
    }
}

AutomationClip::AutomationClip(double startTime, double duration)
    : Clip(ClipType::Automation, startTime, duration)
{
}

void AutomationClip::addPoint(double time, float value) {
    m_points.emplace_back(time, value);
    std::sort(m_points.begin(), m_points.end(), 
        [](const AutomationPoint& a, const AutomationPoint& b) {
            return a.time < b.time;
        });
}

void AutomationClip::removePoint(size_t index) {
    if (index < m_points.size()) {
        m_points.erase(m_points.begin() + index);
    }
}

void AutomationClip::clearPoints() {
    m_points.clear();
}

float AutomationClip::getValueAtTime(double time) const {
    if (m_points.empty()) return 0.0f;
    if (m_points.size() == 1) return m_points[0].value;
    
    if (time <= m_points.front().time) return m_points.front().value;
    if (time >= m_points.back().time) return m_points.back().value;
    
    for (size_t i = 0; i < m_points.size() - 1; ++i) {
        if (time >= m_points[i].time && time <= m_points[i + 1].time) {
            double t = (time - m_points[i].time) / (m_points[i + 1].time - m_points[i].time);
            return m_points[i].value + t * (m_points[i + 1].value - m_points[i].value);
        }
    }
    
    return m_points.back().value;
}

} // namespace OmegaDAW
