# Mixer and Routing System

## Overview

The Omega DAW Mixer and Routing System provides professional-grade audio mixing capabilities with flexible routing options. It consists of two main components:

1. **Mixer System** - High-level mixing with buses, sends, and effects
2. **Router System** - Low-level node-based audio routing with graph processing

## Mixer System

### Features

- **Multiple Bus Types**
  - Audio buses for tracks
  - Group buses for submixing
  - Auxiliary buses for effects sends
  - Master bus for final output

- **Bus Controls**
  - Volume (gain) control
  - Pan control (-1.0 to +1.0)
  - Mute/Solo functionality
  - Pre/post-fader sends

- **Effects Chain**
  - Insert multiple effects per bus
  - Effects processed in series
  - Enable/disable individual effects
  - Automatic routing through effect chain

- **Send/Return System**
  - Send audio from any bus to any other bus
  - Adjustable send levels
  - Multiple sends per bus
  - Parallel processing support

### Usage Example

```cpp
#include "Mixer.h"

// Create mixer
omega::Mixer mixer;
mixer.initialize(44100, 512);

// Add buses
int track1Bus = mixer.addBus("Track 1", omega::ChannelType::Audio);
int track2Bus = mixer.addBus("Track 2", omega::ChannelType::Audio);
int reverbBus = mixer.addBus("Reverb", omega::ChannelType::Aux);

// Configure bus
auto bus = mixer.getBus(track1Bus);
bus->setVolume(0.8f);
bus->setPan(-0.3f);  // Pan left

// Add send to reverb
bus->addSend(reverbBus, 0.3f);  // 30% to reverb

// Add effects
auto eq = std::make_shared<MyEQEffect>();
bus->addEffect(eq);

// Process audio
mixer.setBusInput(track1Bus, inputBuffer);
mixer.process();
AudioBuffer output = mixer.getMasterOutput();
```

### MixerBus API

```cpp
class MixerBus {
    // Volume and pan
    void setVolume(float volume);      // 0.0 = silent, 1.0 = unity
    void setPan(float pan);            // -1.0 = left, 0.0 = center, 1.0 = right
    
    // Mute and solo
    void setMute(bool mute);
    void setSolo(bool solo);
    
    // Effects
    void addEffect(std::shared_ptr<Effect> effect);
    void removeEffect(int index);
    std::shared_ptr<Effect> getEffect(int index);
    
    // Sends
    void addSend(int targetBusId, float level);
    void removeSend(int targetBusId);
    void setSendLevel(int targetBusId, float level);
};
```

### Mixer API

```cpp
class Mixer {
    void initialize(int sampleRate, int bufferSize);
    void process();
    void reset();
    
    // Bus management
    int addBus(const std::string& name, ChannelType type);
    void removeBus(int busId);
    std::shared_ptr<MixerBus> getBus(int busId);
    
    // Routing
    void routeAudio(int sourceBusId, int targetBusId, float level);
    void removeRoute(int sourceBusId, int targetBusId);
    
    // I/O
    void setBusInput(int busId, const AudioBuffer& buffer);
    AudioBuffer getMasterOutput();
    
    // Solo mode
    void setSoloMode(bool enabled);
};
```

## Router System

### Features

- **Node-Based Architecture**
  - Input, output, gain, pan, mix, and split nodes
  - Custom node support via AudioNode base class
  - Multi-channel support

- **Connection Graph**
  - Connect any node output to any node input
  - Adjustable connection gain
  - Enable/disable connections
  - Automatic cycle detection

- **Topological Sorting**
  - Automatic processing order calculation
  - Ensures dependencies are processed first
  - Efficient graph traversal

- **Built-in Node Types**
  - InputNode - Audio input
  - OutputNode - Audio output
  - GainNode - Volume adjustment
  - PanNode - Stereo panning
  - MixNode - Multiple input mixing
  - SplitNode - Multiple output splitting

### Usage Example

```cpp
#include "Router.h"

// Create router
omega::Router router;
router.initialize(44100, 512);

// Create nodes
auto input1 = std::make_shared<omega::InputNode>(1, 2);
auto input2 = std::make_shared<omega::InputNode>(2, 2);
auto gain1 = std::make_shared<omega::GainNode>(3);
auto mixer = std::make_shared<omega::MixNode>(4, 2);
auto output = std::make_shared<omega::OutputNode>(5, 2);

// Add to router
router.addNode(input1);
router.addNode(input2);
router.addNode(gain1);
router.addNode(mixer);
router.addNode(output);

// Create connections
router.connect(1, 0, 3, 0);      // input1 -> gain1
router.connect(3, 0, 4, 0);      // gain1 -> mixer input 1
router.connect(2, 0, 4, 1);      // input2 -> mixer input 2
router.connect(4, 0, 5, 0);      // mixer -> output

// Configure nodes
gain1->setGain(0.7f);

// Process
input1->setInputBuffer(buffer1);
input2->setInputBuffer(buffer2);
router.process();
AudioBuffer result = output->getOutputBuffer();
```

### AudioNode API

```cpp
class AudioNode {
    virtual void process() = 0;
    virtual void reset() = 0;
    
    int getId() const;
    int getNumInputs() const;
    int getNumOutputs() const;
    
    AudioBuffer& getInputBuffer(int channel);
    AudioBuffer& getOutputBuffer(int channel);
};
```

### Router API

```cpp
class Router {
    void initialize(int sampleRate, int bufferSize);
    void process();
    void reset();
    
    // Node management
    void addNode(std::shared_ptr<AudioNode> node);
    void removeNode(int nodeId);
    std::shared_ptr<AudioNode> getNode(int nodeId);
    
    // Connections
    void connect(int sourceId, int sourceChannel, int targetId, int targetChannel, float gain);
    void disconnect(int sourceId, int sourceChannel, int targetId, int targetChannel);
    void disconnectAll(int nodeId);
    
    // Connection properties
    void setConnectionGain(int sourceId, int sourceChannel, int targetId, int targetChannel, float gain);
    void setConnectionEnabled(int sourceId, int sourceChannel, int targetId, int targetChannel, bool enabled);
    
    // Query
    std::vector<Connection> getConnectionsFrom(int nodeId) const;
    std::vector<Connection> getConnectionsTo(int nodeId) const;
    bool detectCycle(int sourceId, int targetId) const;
};
```

## Integration with DAW

### Track to Mixer Integration

```cpp
// In Track class
void Track::render(AudioBuffer& output) {
    // Render clips to buffer
    renderClips(output);
    
    // Send to mixer bus
    mixer->setBusInput(mixerBusId_, output);
}
```

### Real-time Processing

```cpp
// In AudioEngine callback
void audioCallback(AudioBuffer& outputBuffer) {
    // Render all tracks
    for (auto& track : tracks) {
        track->render(trackBuffer);
    }
    
    // Process mixer
    mixer.process();
    
    // Get master output
    outputBuffer = mixer.getMasterOutput();
}
```

## Performance Considerations

1. **Buffer Management**
   - Reuse buffers where possible
   - Pre-allocate buffers during initialization
   - Clear buffers before processing

2. **Graph Optimization**
   - Topological sorting ensures efficient processing
   - Cycle detection prevents infinite loops
   - Disabled connections are skipped

3. **Solo Mode**
   - Only soloed buses are processed
   - Reduces CPU usage when working on specific tracks

4. **Send Optimization**
   - Sends are processed after bus effects
   - Multiple sends processed in parallel

## Future Enhancements

- Sidechain routing support
- Pre/post-fader send options
- Bus grouping and nesting
- Metering and analysis
- Automation recording and playback
- Save/load mixer configurations
- MIDI-controlled parameters
- External hardware integration

## Related Components

- **AudioBuffer** - Audio data storage
- **Effect** - Audio effect processing
- **Track** - Track audio rendering
- **AudioEngine** - Real-time audio processing
