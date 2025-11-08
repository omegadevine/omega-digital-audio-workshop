#include "MIDISequencer.h"
#include <algorithm>
#include <cmath>

namespace OmegaDAW {

// MIDIPattern Implementation
MIDIPattern::MIDIPattern()
    : length_(4.0)
    , looping_(false) {
}

void MIDIPattern::addNote(const MIDINote& note) {
    notes_.push_back(note);
}

void MIDIPattern::removeNote(int index) {
    if (index >= 0 && index < static_cast<int>(notes_.size())) {
        notes_.erase(notes_.begin() + index);
    }
}

void MIDIPattern::clearNotes() {
    notes_.clear();
}

void MIDIPattern::getMessagesInRange(double startTime, double endTime, MIDIBuffer& buffer) const {
    for (const auto& note : notes_) {
        if (note.startTime >= startTime && note.startTime < endTime) {
            MIDIMessage noteOn = MIDIMessage::noteOn(note.channel, note.noteNumber, note.velocity);
            noteOn.setTimestamp(note.startTime);
            buffer.addMessage(noteOn);
        }
        
        double noteEndTime = note.startTime + note.duration;
        if (noteEndTime >= startTime && noteEndTime < endTime) {
            MIDIMessage noteOff = MIDIMessage::noteOff(note.channel, note.noteNumber);
            noteOff.setTimestamp(noteEndTime);
            buffer.addMessage(noteOff);
        }
    }
}

void MIDIPattern::quantize(double gridSize) {
    for (auto& note : notes_) {
        note.startTime = std::round(note.startTime / gridSize) * gridSize;
        note.duration = std::round(note.duration / gridSize) * gridSize;
        if (note.duration < gridSize) {
            note.duration = gridSize;
        }
    }
}

void MIDIPattern::transpose(int semitones) {
    for (auto& note : notes_) {
        int newNote = note.noteNumber + semitones;
        if (newNote >= 0 && newNote <= 127) {
            note.noteNumber = newNote;
        }
    }
}

// MIDISequencer Implementation
MIDISequencer::MIDISequencer()
    : tempo_(120.0)
    , timeSignatureNum_(4)
    , timeSignatureDenom_(4)
    , isRecording_(false)
    , recordStartTime_(0.0) {
}

void MIDISequencer::addClip(std::shared_ptr<MIDIPattern> clip, double startTime) {
    clips_.push_back({clip, startTime});
}

void MIDISequencer::removeClip(int index) {
    if (index >= 0 && index < static_cast<int>(clips_.size())) {
        clips_.erase(clips_.begin() + index);
    }
}

void MIDISequencer::clearClips() {
    clips_.clear();
}

void MIDISequencer::process(double startTime, double endTime, MIDIBuffer& outputBuffer) {
    for (const auto& instance : clips_) {
        double clipStartTime = instance.startTime;
        double clipEndTime = clipStartTime + instance.clip->getLength();
        
        if (instance.clip->isLooping()) {
            double clipLength = instance.clip->getLength();
            if (clipLength > 0.0) {
                double relativeStart = std::max(0.0, startTime - clipStartTime);
                double relativeEnd = endTime - clipStartTime;
                
                int startLoop = static_cast<int>(std::floor(relativeStart / clipLength));
                int endLoop = static_cast<int>(std::ceil(relativeEnd / clipLength));
                
                for (int loop = startLoop; loop <= endLoop; ++loop) {
                    double loopOffset = clipStartTime + loop * clipLength;
                    double loopStart = std::max(startTime, loopOffset);
                    double loopEnd = std::min(endTime, loopOffset + clipLength);
                    
                    if (loopStart < loopEnd) {
                        MIDIBuffer tempBuffer;
                        instance.clip->getMessagesInRange(
                            loopStart - loopOffset,
                            loopEnd - loopOffset,
                            tempBuffer
                        );
                        
                        for (int i = 0; i < tempBuffer.getNumMessages(); ++i) {
                            MIDIMessage msg = tempBuffer.getMessage(i);
                            msg.setTimestamp(msg.getTimestamp() + loopOffset);
                            outputBuffer.addMessage(msg);
                        }
                    }
                }
            }
        } else {
            if (startTime < clipEndTime && endTime > clipStartTime) {
                double relativeStart = std::max(0.0, startTime - clipStartTime);
                double relativeEnd = std::min(instance.clip->getLength(), endTime - clipStartTime);
                
                MIDIBuffer tempBuffer;
                instance.clip->getMessagesInRange(relativeStart, relativeEnd, tempBuffer);
                
                for (int i = 0; i < tempBuffer.getNumMessages(); ++i) {
                    MIDIMessage msg = tempBuffer.getMessage(i);
                    msg.setTimestamp(msg.getTimestamp() + clipStartTime);
                    outputBuffer.addMessage(msg);
                }
            }
        }
    }
    
    outputBuffer.sortByTimestamp();
}

void MIDISequencer::setTempo(double bpm) {
    if (bpm > 0.0) {
        tempo_ = bpm;
    }
}

void MIDISequencer::setTimeSignature(int numerator, int denominator) {
    if (numerator > 0 && denominator > 0) {
        timeSignatureNum_ = numerator;
        timeSignatureDenom_ = denominator;
    }
}

double MIDISequencer::beatsToSeconds(double beats) const {
    return (beats / tempo_) * 60.0;
}

double MIDISequencer::secondsToBeats(double seconds) const {
    return (seconds * tempo_) / 60.0;
}

void MIDISequencer::recordMessage(const MIDIMessage& message) {
    if (isRecording_ && recordingClip_) {
        // Convert to note format if it's a note on message
        if (message.isNoteOn()) {
            double timestamp = message.getTimestamp() - recordStartTime_;
            MIDINote note(
                message.getChannel(),
                message.getNoteNumber(),
                message.getVelocity(),
                timestamp,
                0.25 // Default duration, will be updated on note off
            );
            recordingClip_->addNote(note);
        }
        // Handle note off to update duration
        else if (message.isNoteOff()) {
            // Find matching note and update duration
            int numNotes = recordingClip_->getNumNotes();
            for (int i = numNotes - 1; i >= 0; --i) {
                MIDINote& note = recordingClip_->getNote(i);
                if (note.channel == message.getChannel() && 
                    note.noteNumber == message.getNoteNumber()) {
                    double endTime = message.getTimestamp() - recordStartTime_;
                    note.duration = endTime - note.startTime;
                    break;
                }
            }
        }
    }
}

std::shared_ptr<MIDIPattern> MIDISequencer::stopRecording() {
    isRecording_ = false;
    auto clip = recordingClip_;
    recordingClip_ = nullptr;
    return clip;
}

} // namespace OmegaDAW
