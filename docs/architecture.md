# Omega DAW Architecture

## Overview

Omega Digital Audio Workshop is designed with a modular, layered architecture for maximum flexibility and maintainability.

## Core Components

### 1. Audio Engine Layer
- **AudioEngine**: Central audio processing coordinator
- **AudioBuffer**: Multi-channel audio buffer management
- **Transport**: Playback, recording, and timeline control
- **MixerChannel**: Audio routing and effects processing

### 2. Track Management
- **Track**: Individual audio/MIDI track representation
- **Project**: Container for tracks and project settings
- **TrackType**: Audio, MIDI, and Master track types

### 3. MIDI System
- **MIDIMessage**: Individual MIDI message representation
- **MIDIBuffer**: Collection and timing of MIDI messages
- Support for Note On/Off, CC, Pitch Bend, etc.

### 4. Effects System
- **Effect**: Base interface for audio effects
- **MixerChannel**: Hosts effect chains
- Pre/post gain staging

### 5. GUI Layer
- **MainWindow**: Primary application window
- Transport controls
- Track list view
- Timeline/arrangement view
- Mixer view

## Audio Processing Flow

```
Input Device → AudioEngine → Track Processing → Mixer → Output Device
                    ↓
                MIDI Input → Track Recording → MIDI Buffer
```

## Threading Model

- **Audio Thread**: Real-time audio processing (high priority)
- **GUI Thread**: User interface updates
- **File I/O Thread**: Project loading/saving, audio file operations

## Data Flow

1. Audio input captured by AudioEngine
2. Routed to individual tracks
3. Track processing (effects, volume, pan)
4. Mixed in master channel
5. Output to audio device

## Plugin Architecture (Future)

- VST3 plugin hosting
- Effect and instrument plugins
- MIDI effect plugins
- Custom plugin format

## File Formats

### Project Files (.odaw)
- Binary format for fast loading
- Contains all project settings
- Track configurations
- Plugin states
- Automation data

### Audio Files
- WAV (primary format)
- FLAC (lossless compression)
- MP3 (import only)
- OGG (import/export)

## Performance Considerations

- Lock-free audio processing where possible
- Efficient buffer management
- Minimal allocations in audio thread
- SIMD optimization for DSP operations
- Multi-core utilization for track processing

## Extensibility

- Plugin interface for custom effects
- Custom GUI themes
- Scripting support (future)
- External controller integration

## Dependencies

- **PortAudio**: Cross-platform audio I/O
- **RtMidi**: MIDI support
- **SQLite**: Project database
- **Dear ImGui**: GUI framework (planned)
- **JUCE** (optional): Advanced audio features
