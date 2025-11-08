#include "MIDIFile.h"
#include <fstream>
#include <iostream>
#include <cmath>

#ifdef _WIN32
#include <stdlib.h>
#define BSWAP32(x) _byteswap_ulong(x)
#define BSWAP16(x) _byteswap_ushort(x)
#else
#define BSWAP32(x) __builtin_bswap32(x)
#define BSWAP16(x) __builtin_bswap16(x)
#endif

namespace OmegaDAW {

MIDIFile::MIDIFile()
    : format_(MIDIFileFormat::MultiTrack)
    , ticksPerQuarterNote_(480)
    , tempo_(120.0) {
}

bool MIDIFile::load(const std::string& filename) {
    return readMIDIFile(filename);
}

bool MIDIFile::save(const std::string& filename) {
    return writeMIDIFile(filename);
}

void MIDIFile::clear() {
    tracks_.clear();
    format_ = MIDIFileFormat::MultiTrack;
    ticksPerQuarterNote_ = 480;
    tempo_ = 120.0;
}

void MIDIFile::setTempo(double bpm) {
    if (bpm > 0.0) {
        tempo_ = bpm;
    }
}

void MIDIFile::addTrack(const MIDITrackData& track) {
    tracks_.push_back(track);
}

/*
void MIDIFile::convertToClips(std::vector<std::shared_ptr<MIDIClip>>& clips) const {
    for (const auto& track : tracks_) {
        auto clip = std::make_shared<MIDIClip>();
        
        std::vector<std::pair<int, double>> noteOns;
        
        for (const auto& msg : track.messages) {
            if (msg.isNoteOn()) {
                noteOns.push_back({msg.getNoteNumber(), msg.getTimestamp()});
            } else if (msg.isNoteOff()) {
                for (auto it = noteOns.begin(); it != noteOns.end(); ++it) {
                    if (it->first == msg.getNoteNumber()) {
                        double duration = msg.getTimestamp() - it->second;
                        MIDINote note(
                            msg.getChannel(),
                            msg.getNoteNumber(),
                            64, // Default velocity
                            it->second,
                            duration
                        );
                        clip->addNote(note);
                        noteOns.erase(it);
                        break;
                    }
                }
            }
        }
        
        // Handle any remaining note ons without note offs
        for (const auto& noteOn : noteOns) {
            MIDINote note(track.channel, noteOn.first, 64, noteOn.second, 0.25);
            clip->addNote(note);
        }
        
        if (clip->getNumNotes() > 0) {
            clips.push_back(clip);
        }
    }
}

void MIDIFile::loadFromClips(const std::vector<std::shared_ptr<MIDIClip>>& clips) {
    tracks_.clear();
    
    for (const auto& clip : clips) {
        MIDITrackData track;
        track.name = "MIDI Track";
        track.channel = 0;
        
        for (int i = 0; i < clip->getNumNotes(); ++i) {
            const MIDINote& note = clip->getNote(i);
            
            MIDIMessage noteOn = MIDIMessage::noteOn(
                note.channel, note.noteNumber, note.velocity
            );
            noteOn.setTimestamp(note.startTime);
            track.messages.push_back(noteOn);
            
            MIDIMessage noteOff = MIDIMessage::noteOff(
                note.channel, note.noteNumber
            );
            noteOff.setTimestamp(note.startTime + note.duration);
            track.messages.push_back(noteOff);
        }
        
        tracks_.push_back(track);
    }
}
*/

double MIDIFile::ticksToSeconds(int ticks) const {
    double beatsPerSecond = tempo_ / 60.0;
    double secondsPerBeat = 1.0 / beatsPerSecond;
    double secondsPerTick = secondsPerBeat / ticksPerQuarterNote_;
    return ticks * secondsPerTick;
}

int MIDIFile::secondsToTicks(double seconds) const {
    double beatsPerSecond = tempo_ / 60.0;
    double secondsPerBeat = 1.0 / beatsPerSecond;
    double secondsPerTick = secondsPerBeat / ticksPerQuarterNote_;
    return static_cast<int>(std::round(seconds / secondsPerTick));
}

bool MIDIFile::readMIDIFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open MIDI file: " << filename << std::endl;
        return false;
    }
    
    // Read MIDI header chunk
    char headerChunk[4];
    file.read(headerChunk, 4);
    if (std::string(headerChunk, 4) != "MThd") {
        std::cerr << "Invalid MIDI file header" << std::endl;
        return false;
    }
    
    // Read header length (should be 6)
    uint32_t headerLength = 0;
    file.read(reinterpret_cast<char*>(&headerLength), 4);
    headerLength = BSWAP32(headerLength);
    
    // Read format
    uint16_t formatType = 0;
    file.read(reinterpret_cast<char*>(&formatType), 2);
    formatType = BSWAP16(formatType);
    format_ = static_cast<MIDIFileFormat>(formatType);
    
    // Read number of tracks
    uint16_t numTracks = 0;
    file.read(reinterpret_cast<char*>(&numTracks), 2);
    numTracks = BSWAP16(numTracks);
    
    // Read ticks per quarter note
    uint16_t division = 0;
    file.read(reinterpret_cast<char*>(&division), 2);
    division = BSWAP16(division);
    ticksPerQuarterNote_ = division;
    
    std::cout << "MIDI file loaded: Format " << static_cast<int>(format_) 
              << ", " << numTracks << " tracks, " 
              << ticksPerQuarterNote_ << " ticks/quarter" << std::endl;
    
    // Note: Full track parsing would be implemented here
    // This is a basic stub that shows the structure
    
    file.close();
    return true;
}

bool MIDIFile::writeMIDIFile(const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to create MIDI file: " << filename << std::endl;
        return false;
    }
    
    // Write MIDI header
    file.write("MThd", 4);
    
    uint32_t headerLength = BSWAP32(6);
    file.write(reinterpret_cast<const char*>(&headerLength), 4);
    
    uint16_t formatType = BSWAP16(static_cast<uint16_t>(format_));
    file.write(reinterpret_cast<const char*>(&formatType), 2);
    
    uint16_t numTracks = BSWAP16(static_cast<uint16_t>(tracks_.size()));
    file.write(reinterpret_cast<const char*>(&numTracks), 2);
    
    uint16_t division = BSWAP16(static_cast<uint16_t>(ticksPerQuarterNote_));
    file.write(reinterpret_cast<const char*>(&division), 2);
    
    // Note: Full track writing would be implemented here
    // This is a basic stub that shows the structure
    
    file.close();
    std::cout << "MIDI file saved: " << filename << std::endl;
    return true;
}

uint32_t MIDIFile::readVariableLength(std::ifstream& file) {
    uint32_t value = 0;
    uint8_t byte;
    
    do {
        file.read(reinterpret_cast<char*>(&byte), 1);
        value = (value << 7) | (byte & 0x7F);
    } while (byte & 0x80);
    
    return value;
}

void MIDIFile::writeVariableLength(std::ofstream& file, uint32_t value) {
    uint32_t buffer = value & 0x7F;
    
    while ((value >>= 7) > 0) {
        buffer <<= 8;
        buffer |= 0x80;
        buffer += (value & 0x7F);
    }
    
    while (true) {
        uint8_t byte = static_cast<uint8_t>(buffer & 0xFF);
        file.write(reinterpret_cast<const char*>(&byte), 1);
        if (buffer & 0x80) {
            buffer >>= 8;
        } else {
            break;
        }
    }
}

} // namespace OmegaDAW
