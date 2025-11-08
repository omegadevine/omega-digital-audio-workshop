#include "Arrangement.h"
#include <algorithm>
#include <cmath>

namespace OmegaDAW {

Arrangement::Arrangement()
    : m_loopEnabled(false)
    , m_loopStart(0.0)
    , m_loopEnd(0.0)
    , m_gridSize(0.25)
    , m_snapToGrid(true)
    , m_totalDuration(300.0)
{
    m_timeSignatureChanges.emplace_back(0.0, 4, 4);
}

void Arrangement::addClip(size_t trackIndex, std::shared_ptr<Clip> clip) {
    if (!clip) return;
    
    ClipEntry entry;
    entry.trackIndex = trackIndex;
    entry.clip = clip;
    
    m_clips.push_back(entry);
    
    std::sort(m_clips.begin(), m_clips.end(), 
        [](const ClipEntry& a, const ClipEntry& b) {
            if (a.trackIndex != b.trackIndex) {
                return a.trackIndex < b.trackIndex;
            }
            return a.clip->getStartTime() < b.clip->getStartTime();
        });
}

void Arrangement::removeClip(size_t trackIndex, size_t clipIndex) {
    auto clips = getClipsOnTrack(trackIndex);
    if (clipIndex >= clips.size()) return;
    
    auto clipToRemove = clips[clipIndex];
    
    m_clips.erase(
        std::remove_if(m_clips.begin(), m_clips.end(),
            [&](const ClipEntry& entry) {
                return entry.trackIndex == trackIndex && entry.clip == clipToRemove;
            }),
        m_clips.end()
    );
}

void Arrangement::moveClip(size_t trackIndex, size_t clipIndex, double newStartTime) {
    auto clips = getClipsOnTrack(trackIndex);
    if (clipIndex >= clips.size()) return;
    
    if (m_snapToGrid) {
        newStartTime = snapTimeToGrid(newStartTime);
    }
    
    clips[clipIndex]->setStartTime(newStartTime);
    
    std::sort(m_clips.begin(), m_clips.end(), 
        [](const ClipEntry& a, const ClipEntry& b) {
            if (a.trackIndex != b.trackIndex) {
                return a.trackIndex < b.trackIndex;
            }
            return a.clip->getStartTime() < b.clip->getStartTime();
        });
}

void Arrangement::resizeClip(size_t trackIndex, size_t clipIndex, double newDuration) {
    auto clips = getClipsOnTrack(trackIndex);
    if (clipIndex >= clips.size()) return;
    
    if (m_snapToGrid) {
        newDuration = snapTimeToGrid(newDuration);
    }
    
    clips[clipIndex]->setDuration(newDuration);
}

void Arrangement::splitClip(size_t trackIndex, size_t clipIndex, double splitTime) {
    auto clips = getClipsOnTrack(trackIndex);
    if (clipIndex >= clips.size()) return;
    
    auto originalClip = clips[clipIndex];
    if (!originalClip->isInRange(splitTime)) return;
    
    double firstDuration = splitTime - originalClip->getStartTime();
    double secondStart = splitTime;
    double secondDuration = originalClip->getEndTime() - splitTime;
    
    originalClip->setDuration(firstDuration);
    
    std::shared_ptr<Clip> newClip;
    
    if (originalClip->getType() == ClipType::Audio) {
        auto audioClip = std::static_pointer_cast<AudioClip>(originalClip);
        auto newAudioClip = std::make_shared<AudioClip>(secondStart, secondDuration);
        newAudioClip->setAudioData(audioClip->getAudioData());
        newAudioClip->setSourceFile(audioClip->getSourceFile());
        newAudioClip->setOffset(audioClip->getOffset() + firstDuration);
        newClip = newAudioClip;
    } else if (originalClip->getType() == ClipType::MIDI) {
        auto midiClip = std::static_pointer_cast<MIDIClip>(originalClip);
        auto newMidiClip = std::make_shared<MIDIClip>(secondStart, secondDuration);
        auto notes = midiClip->getNotesInRange(splitTime, originalClip->getEndTime());
        for (auto& note : notes) {
            note.timestamp -= firstDuration;
            newMidiClip->addNote(note);
        }
        newClip = newMidiClip;
    }
    
    if (newClip) {
        newClip->setGain(originalClip->getGain());
        newClip->setName(originalClip->getName());
        newClip->setColor(originalClip->getColor());
        addClip(trackIndex, newClip);
    }
}

std::shared_ptr<Clip> Arrangement::duplicateClip(size_t trackIndex, size_t clipIndex) {
    auto clips = getClipsOnTrack(trackIndex);
    if (clipIndex >= clips.size()) return nullptr;
    
    auto originalClip = clips[clipIndex];
    std::shared_ptr<Clip> newClip;
    
    if (originalClip->getType() == ClipType::Audio) {
        auto audioClip = std::static_pointer_cast<AudioClip>(originalClip);
        auto newAudioClip = std::make_shared<AudioClip>(
            originalClip->getStartTime() + originalClip->getDuration(),
            originalClip->getDuration()
        );
        newAudioClip->setAudioData(audioClip->getAudioData());
        newAudioClip->setSourceFile(audioClip->getSourceFile());
        newAudioClip->setOffset(audioClip->getOffset());
        newAudioClip->setPitch(audioClip->getPitch());
        newClip = newAudioClip;
    } else if (originalClip->getType() == ClipType::MIDI) {
        auto midiClip = std::static_pointer_cast<MIDIClip>(originalClip);
        auto newMidiClip = std::make_shared<MIDIClip>(
            originalClip->getStartTime() + originalClip->getDuration(),
            originalClip->getDuration()
        );
        for (const auto& note : midiClip->getNotes()) {
            newMidiClip->addNote(note);
        }
        newClip = newMidiClip;
    }
    
    if (newClip) {
        newClip->setGain(originalClip->getGain());
        newClip->setName(originalClip->getName());
        newClip->setColor(originalClip->getColor());
        addClip(trackIndex, newClip);
    }
    
    return newClip;
}

std::vector<std::shared_ptr<Clip>> Arrangement::getClipsOnTrack(size_t trackIndex) const {
    std::vector<std::shared_ptr<Clip>> result;
    for (const auto& entry : m_clips) {
        if (entry.trackIndex == trackIndex) {
            result.push_back(entry.clip);
        }
    }
    return result;
}

std::vector<std::shared_ptr<Clip>> Arrangement::getClipsInTimeRange(size_t trackIndex, double startTime, double endTime) const {
    std::vector<std::shared_ptr<Clip>> result;
    for (const auto& entry : m_clips) {
        if (entry.trackIndex == trackIndex) {
            auto clip = entry.clip;
            if (clip->getEndTime() > startTime && clip->getStartTime() < endTime) {
                result.push_back(clip);
            }
        }
    }
    return result;
}

std::shared_ptr<Clip> Arrangement::getClipAt(size_t trackIndex, double time) const {
    for (const auto& entry : m_clips) {
        if (entry.trackIndex == trackIndex && entry.clip->isInRange(time)) {
            return entry.clip;
        }
    }
    return nullptr;
}

void Arrangement::setLoop(bool enabled, double loopStart, double loopEnd) {
    m_loopEnabled = enabled;
    m_loopStart = loopStart;
    m_loopEnd = loopEnd;
}

double Arrangement::snapTimeToGrid(double time) const {
    if (!m_snapToGrid || m_gridSize <= 0.0) return time;
    return std::round(time / m_gridSize) * m_gridSize;
}

void Arrangement::clear() {
    m_clips.clear();
    m_markers.clear();
}

void Arrangement::addMarker(const Marker& marker) {
    m_markers.push_back(marker);
    std::sort(m_markers.begin(), m_markers.end(), 
        [](const Marker& a, const Marker& b) {
            return a.time < b.time;
        });
}

void Arrangement::removeMarker(size_t index) {
    if (index < m_markers.size()) {
        m_markers.erase(m_markers.begin() + index);
    }
}

void Arrangement::addTimeSignatureChange(const TimeSignatureChange& change) {
    m_timeSignatureChanges.push_back(change);
    std::sort(m_timeSignatureChanges.begin(), m_timeSignatureChanges.end(), 
        [](const TimeSignatureChange& a, const TimeSignatureChange& b) {
            return a.time < b.time;
        });
}

void Arrangement::removeTimeSignatureChange(size_t index) {
    if (index < m_timeSignatureChanges.size() && m_timeSignatureChanges.size() > 1) {
        m_timeSignatureChanges.erase(m_timeSignatureChanges.begin() + index);
    }
}

Arrangement::TimeSignatureChange Arrangement::getTimeSignatureAt(double time) const {
    for (auto it = m_timeSignatureChanges.rbegin(); it != m_timeSignatureChanges.rend(); ++it) {
        if (time >= it->time) {
            return *it;
        }
    }
    return m_timeSignatureChanges.front();
}

void Arrangement::initialize() {
    // Initialize arrangement
}

void Arrangement::start() {
    // Start playback
}

void Arrangement::stop() {
    // Stop playback
}

void Arrangement::shutdown() {
    clear();
}

AudioBuffer Arrangement::renderAtPosition(double position) {
    // Create empty buffer for now
    AudioBuffer buffer;
    return buffer;
}

void Arrangement::loadFromProject(Project* project) {
    // Load arrangement from project
    if (project) {
        // Parse arrangement data
    }
}

std::string Arrangement::serialize() const {
    // Serialize arrangement to string
    return "";
}

} // namespace OmegaDAW
