#ifndef OMEGA_DAW_TRACK_H
#define OMEGA_DAW_TRACK_H

#include "AudioBuffer.h"
#include <string>
#include <vector>
#include <memory>

namespace omega {

enum class TrackType {
    Audio,
    MIDI,
    Master
};

class Track {
public:
    Track(const std::string& name, TrackType type);
    ~Track() = default;

    void process(AudioBuffer& buffer, int numSamples);
    
    void setVolume(float volume);
    float getVolume() const { return volume_; }
    
    void setPan(float pan);
    float getPan() const { return pan_; }
    
    void setMute(bool mute);
    bool isMuted() const { return muted_; }
    
    void setSolo(bool solo);
    bool isSoloed() const { return soloed_; }
    
    void setRecordEnabled(bool enabled);
    bool isRecordEnabled() const { return recordEnabled_; }
    
    const std::string& getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }
    
    TrackType getType() const { return type_; }
    
    int getTrackIndex() const { return trackIndex_; }
    void setTrackIndex(int index) { trackIndex_ = index; }

private:
    std::string name_;
    TrackType type_;
    int trackIndex_;
    
    float volume_;
    float pan_;
    bool muted_;
    bool soloed_;
    bool recordEnabled_;
    
    AudioBuffer trackBuffer_;
};

} // namespace omega

#endif // OMEGA_DAW_TRACK_H
