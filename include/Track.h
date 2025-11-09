#ifndef OMEGA_DAW_TRACK_H
#define OMEGA_DAW_TRACK_H

#include "AudioBuffer.h"
#include "Clip.h"
#include "MIDISynthesizer.h"
#include <string>
#include <vector>
#include <memory>

namespace OmegaDAW {

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
    void processAtTime(AudioBuffer& buffer, int numSamples, double currentTime, double sampleRate);
    
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
    
    // Clip management
    void addClip(std::shared_ptr<Clip> clip);
    void removeClip(size_t index);
    void clearClips();
    const std::vector<std::shared_ptr<Clip>>& getClips() const { return clips_; }
    std::vector<std::shared_ptr<Clip>> getClipsInRange(double startTime, double endTime) const;
    std::shared_ptr<Clip> getClipAt(double time) const;
    
    // MIDI synthesizer for MIDI tracks
    std::shared_ptr<MIDISynthesizer> getSynthesizer() const { return synthesizer_; }
    void setSynthesizer(std::shared_ptr<MIDISynthesizer> synth) { synthesizer_ = synth; }

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
    std::vector<std::shared_ptr<Clip>> clips_;
    
    // MIDI synthesizer for MIDI tracks
    std::shared_ptr<MIDISynthesizer> synthesizer_;
};

} // namespace OmegaDAW

#endif // OMEGA_DAW_TRACK_H
