#include "MIDIMessage.h"
#include <algorithm>

namespace omega {

MIDIMessage::MIDIMessage()
    : status_(0)
    , data1_(0)
    , data2_(0)
    , timestamp_(0.0) {
}

MIDIMessage::MIDIMessage(uint8_t status, uint8_t data1, uint8_t data2)
    : status_(status)
    , data1_(data1)
    , data2_(data2)
    , timestamp_(0.0) {
}

MIDIMessage MIDIMessage::noteOn(int channel, int noteNumber, uint8_t velocity) {
    return MIDIMessage(
        static_cast<uint8_t>(MIDIMessageType::NoteOn) | (channel & 0x0F),
        static_cast<uint8_t>(noteNumber & 0x7F),
        velocity & 0x7F
    );
}

MIDIMessage MIDIMessage::noteOff(int channel, int noteNumber, uint8_t velocity) {
    return MIDIMessage(
        static_cast<uint8_t>(MIDIMessageType::NoteOff) | (channel & 0x0F),
        static_cast<uint8_t>(noteNumber & 0x7F),
        velocity & 0x7F
    );
}

MIDIMessage MIDIMessage::controlChange(int channel, int controllerNumber, int value) {
    return MIDIMessage(
        static_cast<uint8_t>(MIDIMessageType::ControlChange) | (channel & 0x0F),
        static_cast<uint8_t>(controllerNumber & 0x7F),
        static_cast<uint8_t>(value & 0x7F)
    );
}

MIDIMessage MIDIMessage::pitchBend(int channel, int value) {
    uint8_t lsb = value & 0x7F;
    uint8_t msb = (value >> 7) & 0x7F;
    return MIDIMessage(
        static_cast<uint8_t>(MIDIMessageType::PitchBend) | (channel & 0x0F),
        lsb,
        msb
    );
}

bool MIDIMessage::isNoteOn() const {
    return (status_ & 0xF0) == static_cast<uint8_t>(MIDIMessageType::NoteOn) && data2_ > 0;
}

bool MIDIMessage::isNoteOff() const {
    return (status_ & 0xF0) == static_cast<uint8_t>(MIDIMessageType::NoteOff) ||
           ((status_ & 0xF0) == static_cast<uint8_t>(MIDIMessageType::NoteOn) && data2_ == 0);
}

bool MIDIMessage::isControlChange() const {
    return (status_ & 0xF0) == static_cast<uint8_t>(MIDIMessageType::ControlChange);
}

bool MIDIMessage::isPitchBend() const {
    return (status_ & 0xF0) == static_cast<uint8_t>(MIDIMessageType::PitchBend);
}

int MIDIMessage::getChannel() const {
    return status_ & 0x0F;
}

int MIDIMessage::getNoteNumber() const {
    return data1_;
}

int MIDIMessage::getVelocity() const {
    return data2_;
}

int MIDIMessage::getControllerNumber() const {
    return data1_;
}

int MIDIMessage::getControllerValue() const {
    return data2_;
}

int MIDIMessage::getPitchBendValue() const {
    return (data2_ << 7) | data1_;
}

MIDIBuffer::MIDIBuffer() {
}

void MIDIBuffer::addMessage(const MIDIMessage& message) {
    messages_.push_back(message);
}

void MIDIBuffer::clear() {
    messages_.clear();
}

void MIDIBuffer::sortByTimestamp() {
    std::sort(messages_.begin(), messages_.end(), 
        [](const MIDIMessage& a, const MIDIMessage& b) {
            return a.getTimestamp() < b.getTimestamp();
        });
}

} // namespace omega
