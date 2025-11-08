#ifndef OMEGA_DAW_MIDI_SEQUENCER_H
#define OMEGA_DAW_MIDI_SEQUENCER_H

#include "MIDIMessage.h"
#include <vector>
#include <memory>
#include <functional>

namespace omega {

struct MIDINote {
    int channel;
    int noteNumber;
    uint8_t velocity;
    double startTime;
    double duration;
    
    MIDINote(int ch, int note, uint8_t vel, double start, double dur)
        : channel(ch), noteNumber(note), velocity(vel), startTime(start), duration(dur) {}
};

class MIDIClip {
public:
    MIDIClip();
    
    void addNote(const MIDINote& note);
    void removeNote(int index);
    void clearNotes();
    
    int getNumNotes() const { return static_cast<int>(notes_.size()); }
    const MIDINote& getNote(int index) const { return notes_[index]; }
    MIDINote& getNote(int index) { return notes_[index]; }
    
    void getMessagesInRange(double startTime, double endTime, MIDIBuffer& buffer) const;
    
    void setLength(double length) { length_ = length; }
    double getLength() const { return length_; }
    
    void setLooping(bool loop) { looping_ = loop; }
    bool isLooping() const { return looping_; }
    
    void quantize(double gridSize);
    void transpose(int semitones);
    
private:
    std::vector<MIDINote> notes_;
    double length_;
    bool looping_;
};

class MIDISequencer {
public:
    MIDISequencer();
    
    void addClip(std::shared_ptr<MIDIClip> clip, double startTime);
    void removeClip(int index);
    void clearClips();
    
    void process(double startTime, double endTime, MIDIBuffer& outputBuffer);
    
    void setTempo(double bpm);
    double getTempo() const { return tempo_; }
    
    void setTimeSignature(int numerator, int denominator);
    int getTimeSignatureNumerator() const { return timeSignatureNum_; }
    int getTimeSignatureDenominator() const { return timeSignatureDenom_; }
    
    double beatsToSeconds(double beats) const;
    double secondsToBeats(double seconds) const;
    
    void setRecording(bool recording) { isRecording_ = recording; }
    bool isRecording() const { return isRecording_; }
    
    void recordMessage(const MIDIMessage& message);
    std::shared_ptr<MIDIClip> stopRecording();
    
private:
    struct ClipInstance {
        std::shared_ptr<MIDIClip> clip;
        double startTime;
    };
    
    std::vector<ClipInstance> clips_;
    double tempo_;
    int timeSignatureNum_;
    int timeSignatureDenom_;
    bool isRecording_;
    std::shared_ptr<MIDIClip> recordingClip_;
    double recordStartTime_;
};

} // namespace omega

#endif // OMEGA_DAW_MIDI_SEQUENCER_H
