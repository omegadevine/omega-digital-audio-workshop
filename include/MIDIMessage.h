#ifndef OMEGA_DAW_MIDI_MESSAGE_H
#define OMEGA_DAW_MIDI_MESSAGE_H

#include <cstdint>
#include <vector>

namespace omega {

enum class MIDIMessageType {
    NoteOff = 0x80,
    NoteOn = 0x90,
    PolyPressure = 0xA0,
    ControlChange = 0xB0,
    ProgramChange = 0xC0,
    ChannelPressure = 0xD0,
    PitchBend = 0xE0,
    SystemExclusive = 0xF0
};

class MIDIMessage {
public:
    MIDIMessage();
    MIDIMessage(uint8_t status, uint8_t data1, uint8_t data2);
    
    static MIDIMessage noteOn(int channel, int noteNumber, uint8_t velocity);
    static MIDIMessage noteOff(int channel, int noteNumber, uint8_t velocity = 0);
    static MIDIMessage controlChange(int channel, int controllerNumber, int value);
    static MIDIMessage pitchBend(int channel, int value);
    
    bool isNoteOn() const;
    bool isNoteOff() const;
    bool isControlChange() const;
    bool isPitchBend() const;
    
    int getChannel() const;
    int getNoteNumber() const;
    int getVelocity() const;
    int getControllerNumber() const;
    int getControllerValue() const;
    int getPitchBendValue() const;
    
    uint8_t getStatus() const { return status_; }
    uint8_t getData1() const { return data1_; }
    uint8_t getData2() const { return data2_; }
    
    double getTimestamp() const { return timestamp_; }
    void setTimestamp(double timestamp) { timestamp_ = timestamp; }

private:
    uint8_t status_;
    uint8_t data1_;
    uint8_t data2_;
    double timestamp_;
};

class MIDIBuffer {
public:
    MIDIBuffer();
    
    void addMessage(const MIDIMessage& message);
    void clear();
    
    int getNumMessages() const { return static_cast<int>(messages_.size()); }
    const MIDIMessage& getMessage(int index) const { return messages_[index]; }
    
    void sortByTimestamp();

private:
    std::vector<MIDIMessage> messages_;
};

} // namespace omega

#endif // OMEGA_DAW_MIDI_MESSAGE_H
