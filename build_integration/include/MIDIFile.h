#ifndef OMEGA_DAW_MIDI_FILE_H
#define OMEGA_DAW_MIDI_FILE_H

#include "MIDIMessage.h"
#include "MIDISequencer.h"
#include <string>
#include <vector>
#include <memory>

namespace OmegaDAW {

enum class MIDIFileFormat {
    SingleTrack = 0,
    MultiTrack = 1,
    MultiSong = 2
};

struct MIDITrackData {
    std::string name;
    std::vector<MIDIMessage> messages;
    int channel;
};

class MIDIFile {
public:
    MIDIFile();
    
    bool load(const std::string& filename);
    bool save(const std::string& filename);
    
    void clear();
    
    void setFormat(MIDIFileFormat format) { format_ = format; }
    MIDIFileFormat getFormat() const { return format_; }
    
    void setTicksPerQuarterNote(int ticks) { ticksPerQuarterNote_ = ticks; }
    int getTicksPerQuarterNote() const { return ticksPerQuarterNote_; }
    
    void setTempo(double bpm);
    double getTempo() const { return tempo_; }
    
    void addTrack(const MIDITrackData& track);
    int getNumTracks() const { return static_cast<int>(tracks_.size()); }
    const MIDITrackData& getTrack(int index) const { return tracks_[index]; }
    MIDITrackData& getTrack(int index) { return tracks_[index]; }
    
    // void convertToClips(std::vector<std::shared_ptr<MIDIClip>>& clips) const;
    // void loadFromClips(const std::vector<std::shared_ptr<MIDIClip>>& clips);
    
    double ticksToSeconds(int ticks) const;
    int secondsToTicks(double seconds) const;
    
private:
    MIDIFileFormat format_;
    int ticksPerQuarterNote_;
    double tempo_;
    std::vector<MIDITrackData> tracks_;
    
    bool readMIDIFile(const std::string& filename);
    bool writeMIDIFile(const std::string& filename);
    
    uint32_t readVariableLength(std::ifstream& file);
    void writeVariableLength(std::ofstream& file, uint32_t value);
};

} // namespace OmegaDAW

#endif // OMEGA_DAW_MIDI_FILE_H
