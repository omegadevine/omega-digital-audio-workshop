# MIDI System Documentation

## Overview
The Omega DAW MIDI system provides comprehensive MIDI handling capabilities including message processing, sequencing, device I/O, and file import/export.

## Components

### 1. MIDIMessage (Base Layer)
**Files:** `MIDIMessage.h`, `MIDIMessage.cpp`

**Features:**
- MIDI message creation and parsing
- Support for Note On/Off, Control Change, Pitch Bend, Program Change, etc.
- Message buffering and timestamp management
- Message type detection and data extraction

**Key Classes:**
- `MIDIMessage`: Represents a single MIDI message
- `MIDIBuffer`: Stores and manages collections of MIDI messages

### 2. MIDISequencer (Sequencing Layer)
**Files:** `MIDISequencer.h`, `MIDISequencer.cpp`

**Features:**
- MIDI clip management with note editing
- Multi-track sequencing with looping support
- Real-time recording with note-on/off tracking
- Tempo and time signature management
- Quantization and transposition tools
- Beat/time conversion utilities

**Key Classes:**
- `MIDINote`: Represents a musical note with duration
- `MIDIClip`: Container for MIDI notes with editing capabilities
- `MIDISequencer`: Manages clips, playback, and recording

### 3. MIDIDevice (Hardware/Virtual I/O Layer)
**Files:** `MIDIDevice.h`, `MIDIDevice.cpp`

**Features:**
- MIDI device enumeration and management
- Input device handling with message callbacks
- Output device control for sending MIDI
- Device scanning and hot-plug support (framework ready)
- Virtual MIDI devices for testing

**Key Classes:**
- `MIDIInputDevice`: Handles incoming MIDI from hardware/virtual devices
- `MIDIOutputDevice`: Sends MIDI to hardware/virtual devices
- `MIDIDeviceManager`: Singleton for managing all MIDI devices
- `MIDIDeviceInfo`: Device metadata structure

### 4. MIDIFile (Import/Export Layer)
**Files:** `MIDIFile.h`, `MIDIFile.cpp`

**Features:**
- Standard MIDI file (.mid) reading and writing
- Format 0, 1, and 2 support
- Conversion between file format and sequencer clips
- Tempo and timing information preservation
- Variable-length quantity encoding/decoding

**Key Classes:**
- `MIDIFile`: Handles MIDI file I/O and format conversion
- `MIDITrackData`: Represents a single MIDI track

## Usage Examples

### Creating MIDI Messages
```cpp
// Create a note on message
MIDIMessage noteOn = MIDIMessage::noteOn(0, 60, 100); // Channel 0, Middle C, velocity 100

// Create a control change
MIDIMessage cc = MIDIMessage::controlChange(0, 7, 127); // Volume to max
```

### Using the Sequencer
```cpp
MIDISequencer sequencer;
sequencer.setTempo(120.0);

// Create a clip
auto clip = std::make_shared<MIDIClip>();
clip->addNote(MIDINote(0, 60, 100, 0.0, 0.5)); // Add middle C

// Add clip to sequencer
sequencer.addClip(clip, 0.0);

// Process MIDI for a time range
MIDIBuffer buffer;
sequencer.process(0.0, 1.0, buffer);
```

### Recording MIDI
```cpp
sequencer.setRecording(true);

// Feed messages to sequencer
MIDIMessage msg = MIDIMessage::noteOn(0, 60, 100);
msg.setTimestamp(currentTime);
sequencer.recordMessage(msg);

// Stop and get the recorded clip
auto recordedClip = sequencer.stopRecording();
```

### Device Management
```cpp
auto& deviceManager = MIDIDeviceManager::getInstance();
deviceManager.scanDevices();

// Get available devices
auto inputs = deviceManager.getInputDevices();
auto outputs = deviceManager.getOutputDevices();

// Open an input device
auto inputDevice = deviceManager.openInputDevice(0);
inputDevice->setMessageCallback([](const MIDIMessage& msg) {
    // Handle incoming MIDI
});

// Send to output device
auto outputDevice = deviceManager.openOutputDevice(0);
outputDevice->sendMessage(MIDIMessage::noteOn(0, 60, 100));
```

### MIDI File I/O
```cpp
MIDIFile midiFile;

// Load from file
if (midiFile.load("song.mid")) {
    // Convert to clips
    std::vector<std::shared_ptr<MIDIClip>> clips;
    midiFile.convertToClips(clips);
    
    // Add clips to sequencer
    for (auto& clip : clips) {
        sequencer.addClip(clip, 0.0);
    }
}

// Save to file
midiFile.loadFromClips(clips);
midiFile.save("output.mid");
```

## Implementation Status

### ✅ Completed
- MIDI message creation and parsing
- Message buffering and sorting
- MIDI clip and note management
- Sequencer with looping support
- Recording infrastructure
- Device management framework
- MIDI file structure parsing
- Clip conversion utilities

### 🚧 Framework Ready (Needs Platform-Specific Code)
- Hardware MIDI device enumeration
- Actual device I/O (currently stubs)
- Complete MIDI file track parsing
- SysEx message handling

### 📋 Future Enhancements
- MIDI learn functionality
- MPE (MIDI Polyphonic Expression) support
- MIDI clock sync
- MTC (MIDI Time Code) support
- Advanced editing (velocity curves, humanization)
- MIDI effect plugins (arpeggiator, chord generator)

## Integration with DAW

The MIDI system integrates with other Omega DAW components:

1. **Audio Engine**: MIDI triggers audio synthesis via virtual instruments
2. **Transport**: Provides timing for MIDI playback and recording
3. **Track System**: Each track can contain MIDI clips
4. **Project**: Saves/loads MIDI data as part of project files

## Next Steps

To complete the MIDI system:

1. **Platform Integration**: Implement actual device I/O using platform APIs:
   - Windows: Use Windows MM API or ASIO
   - macOS: Use CoreMIDI
   - Linux: Use ALSA or JACK

2. **Complete File Parser**: Implement full SMF track parsing including:
   - Meta events (track name, tempo changes, time signature)
   - All channel messages
   - System exclusive data

3. **Virtual Instruments**: Create synthesizers that respond to MIDI

4. **MIDI Effects**: Implement arpeggiators, chord generators, etc.

5. **User Interface**: Build piano roll editor and MIDI controllers

## Architecture Notes

The MIDI system is designed with clean separation of concerns:

- **Message Layer**: Low-level MIDI data representation
- **Sequencer Layer**: Musical note and clip management
- **Device Layer**: Hardware abstraction for I/O
- **File Layer**: Persistence and interchange

This architecture allows each layer to be tested and extended independently.
