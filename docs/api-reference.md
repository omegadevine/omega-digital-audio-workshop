# Omega DAW API Reference

## Core Classes

### AudioEngine

Central audio processing engine.

```cpp
class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();
    
    // Initialization
    bool initialize(int sampleRate = 44100, int bufferSize = 512);
    void shutdown();
    
    // Playback control
    void startPlayback();
    void stopPlayback();
    void pausePlayback();
    bool isPlaying() const;
    
    // Audio properties
    int getSampleRate() const;
    int getBufferSize() const;
    double getCurrentTime() const;
};
```

**Example:**
```cpp
omega::AudioEngine engine;
if (engine.initialize(48000, 256)) {
    engine.startPlayback();
    // Do work...
    engine.stopPlayback();
    engine.shutdown();
}
```

### AudioBuffer

Multi-channel audio buffer for processing.

```cpp
class AudioBuffer {
public:
    AudioBuffer(int numChannels = 2, int numSamples = 0);
    
    // Buffer management
    void resize(int numSamples);
    void clear();
    
    // Data access
    float* getChannelData(int channel);
    const float* getChannelData(int channel) const;
    
    // Sample operations
    void setSample(int channel, int sample, float value);
    float getSample(int channel, int sample) const;
    
    // Buffer operations
    void copyFrom(const AudioBuffer& other);
    void addFrom(const AudioBuffer& other, float gain = 1.0f);
    void applyGain(float gain);
    void applyGainRamp(float startGain, float endGain);
    
    // Properties
    int getNumChannels() const;
    int getNumSamples() const;
};
```

**Example:**
```cpp
omega::AudioBuffer buffer(2, 512);  // Stereo, 512 samples
buffer.clear();

// Fill with audio data
for (int i = 0; i < buffer.getNumSamples(); ++i) {
    buffer.setSample(0, i, sineWave[i]);
    buffer.setSample(1, i, sineWave[i]);
}

// Apply fade out
buffer.applyGainRamp(1.0f, 0.0f);
```

### Track

Represents an audio or MIDI track.

```cpp
enum class TrackType {
    Audio,
    MIDI,
    Master
};

class Track {
public:
    Track(const std::string& name, TrackType type);
    
    // Processing
    void process(AudioBuffer& buffer, int numSamples);
    
    // Mixer controls
    void setVolume(float volume);     // 0.0 to 2.0
    float getVolume() const;
    
    void setPan(float pan);           // -1.0 (left) to 1.0 (right)
    float getPan() const;
    
    void setMute(bool mute);
    bool isMuted() const;
    
    void setSolo(bool solo);
    bool isSoloed() const;
    
    // Recording
    void setRecordEnabled(bool enabled);
    bool isRecordEnabled() const;
    
    // Properties
    const std::string& getName() const;
    void setName(const std::string& name);
    TrackType getType() const;
};
```

**Example:**
```cpp
auto track = std::make_shared<omega::Track>("Vocals", omega::TrackType::Audio);
track->setVolume(0.8f);
track->setPan(-0.3f);  // Pan slightly left
track->setRecordEnabled(true);
```

### Transport

Timeline and playback control.

```cpp
class Transport {
public:
    Transport();
    
    // Transport control
    void play();
    void stop();
    void pause();
    void record();
    
    bool isPlaying() const;
    bool isRecording() const;
    bool isPaused() const;
    
    // Tempo and timing
    void setTempo(double bpm);
    double getTempo() const;
    
    void setTimeSignature(int numerator, int denominator);
    int getTimeSignatureNumerator() const;
    int getTimeSignatureDenominator() const;
    
    // Looping
    void setLooping(bool enabled);
    bool isLooping() const;
    
    void setLoopStart(double beats);
    void setLoopEnd(double beats);
    double getLoopStart() const;
    double getLoopEnd() const;
    
    // Position
    void setPosition(double beats);
    double getPosition() const;
    
    // Audio settings
    void setSampleRate(int sampleRate);
    int getSampleRate() const;
    
    // Callbacks
    using TransportCallback = std::function<void()>;
    void setPlayCallback(TransportCallback callback);
    void setStopCallback(TransportCallback callback);
    void setPauseCallback(TransportCallback callback);
    void setRecordCallback(TransportCallback callback);
};
```

**Example:**
```cpp
omega::Transport transport;
transport.setTempo(120.0);
transport.setTimeSignature(4, 4);
transport.setLooping(true);
transport.setLoopStart(0.0);
transport.setLoopEnd(4.0);

transport.setPlayCallback([]() {
    std::cout << "Playing started" << std::endl;
});

transport.play();
```

### MIDIMessage

MIDI message representation.

```cpp
enum class MIDIMessageType {
    NoteOff,
    NoteOn,
    PolyPressure,
    ControlChange,
    ProgramChange,
    ChannelPressure,
    PitchBend,
    SystemExclusive
};

class MIDIMessage {
public:
    // Factory methods
    static MIDIMessage noteOn(int channel, int noteNumber, uint8_t velocity);
    static MIDIMessage noteOff(int channel, int noteNumber, uint8_t velocity = 0);
    static MIDIMessage controlChange(int channel, int controllerNumber, int value);
    static MIDIMessage pitchBend(int channel, int value);
    
    // Message type queries
    bool isNoteOn() const;
    bool isNoteOff() const;
    bool isControlChange() const;
    bool isPitchBend() const;
    
    // Data access
    int getChannel() const;
    int getNoteNumber() const;
    int getVelocity() const;
    int getControllerNumber() const;
    int getControllerValue() const;
    int getPitchBendValue() const;
    
    double getTimestamp() const;
    void setTimestamp(double timestamp);
};
```

**Example:**
```cpp
// Create note on message (channel 0, note 60 (middle C), velocity 100)
auto noteOn = omega::MIDIMessage::noteOn(0, 60, 100);
noteOn.setTimestamp(0.5);  // 0.5 seconds into the song

// Create note off message
auto noteOff = omega::MIDIMessage::noteOff(0, 60);
noteOff.setTimestamp(1.0);  // 1.0 seconds into the song

// Check message type
if (noteOn.isNoteOn()) {
    std::cout << "Note: " << noteOn.getNoteNumber() 
              << " Velocity: " << noteOn.getVelocity() << std::endl;
}
```

### Project

Project container and management.

```cpp
class Project {
public:
    Project(const std::string& name = "Untitled");
    
    // File operations
    bool save(const std::string& filepath);
    bool load(const std::string& filepath);
    
    // Track management
    void addTrack(std::shared_ptr<Track> track);
    void removeTrack(int index);
    std::shared_ptr<Track> getTrack(int index);
    int getNumTracks() const;
    
    // Properties
    void setName(const std::string& name);
    const std::string& getName() const;
    
    const std::string& getFilePath() const;
    bool isModified() const;
    
    // Transport access
    Transport& getTransport();
    const Transport& getTransport() const;
    
    // Audio settings
    void setSampleRate(int sampleRate);
    int getSampleRate() const;
    
    void setBufferSize(int bufferSize);
    int getBufferSize() const;
};
```

**Example:**
```cpp
omega::Project project("My Song");
project.setSampleRate(48000);
project.setBufferSize(512);

// Add tracks
auto vocals = std::make_shared<omega::Track>("Vocals", omega::TrackType::Audio);
auto drums = std::make_shared<omega::Track>("Drums", omega::TrackType::Audio);
project.addTrack(vocals);
project.addTrack(drums);

// Configure transport
project.getTransport().setTempo(128.0);

// Save project
if (project.save("my_song.odaw")) {
    std::cout << "Project saved successfully" << std::endl;
}
```

### Effect

Base interface for audio effects.

```cpp
class Effect {
public:
    virtual ~Effect() = default;
    
    virtual void process(AudioBuffer& buffer) = 0;
    virtual void reset() = 0;
    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;
};
```

**Example (Custom Effect):**
```cpp
class GainEffect : public omega::Effect {
public:
    GainEffect(float gain = 1.0f) : gain_(gain), enabled_(true) {}
    
    void process(omega::AudioBuffer& buffer) override {
        if (enabled_) {
            buffer.applyGain(gain_);
        }
    }
    
    void reset() override {
        // Reset internal state
    }
    
    bool isEnabled() const override { return enabled_; }
    void setEnabled(bool enabled) override { enabled_ = enabled; }
    
    void setGain(float gain) { gain_ = gain; }
    
private:
    float gain_;
    bool enabled_;
};
```

## Thread Safety

### Real-Time Safe Operations
These operations are safe to call from the audio thread:
- `AudioBuffer::getSample()`
- `AudioBuffer::setSample()`
- `AudioBuffer::applyGain()`
- `Track::process()`
- `Transport::advance()`

### Non-Real-Time Operations
These should only be called from the GUI thread:
- `Project::addTrack()`
- `Project::removeTrack()`
- `Track::setName()`
- File I/O operations

## Best Practices

1. **Always check initialization return values**
2. **Use shared_ptr for tracks and effects**
3. **Clear buffers before processing**
4. **Clamp audio values to prevent clipping**
5. **Avoid allocations in audio callback**
6. **Use lock-free structures for cross-thread communication**
7. **Call shutdown() before destroying AudioEngine**

## Constants

```cpp
namespace omega {
    constexpr int DEFAULT_SAMPLE_RATE = 44100;
    constexpr int DEFAULT_BUFFER_SIZE = 512;
    constexpr int DEFAULT_NUM_CHANNELS = 2;
    constexpr float DEFAULT_VOLUME = 1.0f;
    constexpr float DEFAULT_PAN = 0.0f;
    constexpr double DEFAULT_TEMPO = 120.0;
}
```
