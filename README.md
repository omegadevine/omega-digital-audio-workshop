# Omega Digital Audio Workshop

A professional Digital Audio Workstation (DAW) built from the ground up with real-time audio processing capabilities.

## Current Features

### Audio Engine (Implemented)
- **Real-time Audio Processing**: Low-latency audio I/O using PortAudio
- **Professional Sample Rates**: 44.1kHz, 48kHz, 96kHz, 192kHz support
- **Flexible Buffer Sizes**: 64-2048 samples for latency optimization
- **Multi-channel Support**: Stereo and multi-channel audio routing
- **Audio Metering**: Real-time peak and RMS level monitoring
- **CPU Load Monitoring**: Track processing efficiency

### DSP Components (Implemented)
- **Oscillators**: Sine, Square, Saw, Triangle, Noise waveforms
- **Filters**: Biquad filters (LowPass, HighPass, BandPass, Notch, AllPass)
- **Delay Effect**: Configurable delay time, feedback, and wet/dry mix
- **Reverb Effect**: Room-based reverb with size, damping, and mix controls
- **Audio Graph**: Modular processor chaining for complex signal routing

### MIDI System (Implemented)
- **MIDI I/O**: Device enumeration and port management
- **MIDI Messages**: Note On/Off, Control Change, Program Change, Pitch Bend
- **MIDI Sequencer**: Multi-track recording and playback
- **MIDI File Support**: Standard MIDI File (SMF) import/export
- **Event Scheduling**: Precise timing and quantization

### UI System (Implemented)
- **Main Window**: Application framework with menu system
- **Transport Controls**: Play, Stop, Record buttons with status display
- **Timeline View**: Visual arrangement with grid and ruler
- **Mixer Interface**: Channel strips with faders, meters, and pan controls
- **Control Widgets**: Knobs, sliders, buttons, and custom controls

### Plugin System (Implemented)
- **Plugin Host**: VST/VST3 plugin loading and management
- **Built-in Effects**: Delay, Reverb, Chorus, Phaser, Flanger, Distortion
- **Plugin GUI**: Parameter control and preset management
- **Real-time Processing**: Low-latency effect chains

### Sequencer/Arrangement (Implemented)
- **Multi-track System**: Independent audio and MIDI tracks
- **Clip Management**: Non-destructive audio and MIDI clips
- **Project Structure**: Complete session management
- **Transport Control**: Play, Stop, Record, Loop functionality

### Mixer/Routing (Implemented)
- **Channel Strips**: Volume, pan, solo, mute per channel
- **Send/Return System**: Auxiliary sends for effects
- **Master Bus**: Final output stage with limiting
- **Flexible Routing**: Custom signal paths

### File I/O System (Implemented)
- **Audio Import/Export**: WAV, AIFF, FLAC, MP3, OGG support
- **Project Files**: JSON-based project save/load
- **MIDI File I/O**: Standard MIDI File import/export
- **Recent Files**: Quick access to recent projects
- **Auto-save**: Automatic project backup

### Features (Planned)
- VST/VST3 plugin GUI integration
- Advanced automation editing
- Time-stretching and pitch-shifting
- Comprehensive undo/redo system
- Complete GUI implementation

## System Requirements

- Windows 10/11 (64-bit)
- 4GB RAM minimum (8GB+ recommended)
- Audio interface compatible with WASAPI/DirectSound
- PortAudio library

## Building from Source

### Prerequisites
- **MSYS2** (includes MinGW-w64 compiler)
- CMake 3.15 or higher
- Git

### Quick Start (Windows)

#### 1. Install MSYS2
Download and install from: https://www.msys2.org/

#### 2. Clone the Repository
```bash
git clone https://github.com/omegadevine/omega-digital-audio-workshop.git
cd omega-digital-audio-workshop
```

#### 3. Setup Dependencies (One-Time)
```powershell
.\scripts\setup_dependencies.ps1
```

This script will:
- Locate your MinGW compiler
- Check for required libraries (SDL2, SDL2_ttf, PortAudio)
- Install any missing dependencies via MSYS2
- Create a CMake toolchain file
- Mark dependencies as installed (won't reinstall next time)

#### 4. Build the Project
```powershell
.\scripts\build.ps1
```

Options:
- `-Clean`: Clean build directory before building
- `-Release`: Build in Release mode (default is Debug)
- `-SkipDeps`: Skip dependency check (not recommended)

Examples:
```powershell
# Normal build (incremental)
.\scripts\build.ps1

# Clean build
.\scripts\build.ps1 -Clean

# Release build
.\scripts\build.ps1 -Release

# Complete rebuild with fresh dependencies
.\scripts\setup_dependencies.ps1 -Force -Clean
.\scripts\build.ps1 -Clean
```

### Running the Application
```bash
.\build\bin\OmegaDAW.exe
```

### Dependency Management

The project uses a persistent dependency system to avoid redundant installations:

- Dependencies are installed **once** in your MSYS2 MinGW directory
- A version marker file (`.deps_installed_1.0.0`) tracks installation
- Builds reuse existing dependencies automatically
- No need to reinstall unless you use `-Force` flag

**Why This Matters:**
- Faster builds (no repeated downloads/installs)
- No version conflicts between builds
- No "missing DLL" errors from inconsistent library versions
- Works reliably across multiple build attempts

See [DEPENDENCY_MANAGEMENT.md](DEPENDENCY_MANAGEMENT.md) for detailed information.

### Troubleshooting

#### "Procedure entry point" Errors
This usually means mixing different compiler/library versions. Solution:
```powershell
.\scripts\setup_dependencies.ps1 -Force -Clean
.\scripts\build.ps1 -Clean
```

#### Dependencies Keep Getting "Reinstalled"
Check if `.deps_installed_*` file exists in project root. Don't delete this file.

#### Build Errors About Missing Libraries
```powershell
.\scripts\setup_dependencies.ps1 -Force
```

For more help, see [DEPENDENCY_MANAGEMENT.md](DEPENDENCY_MANAGEMENT.md).

## Audio Engine Demo

The current demo application includes:

1. **List Audio Devices** - View all available audio interfaces
2. **Test Oscillators** - Generate different waveforms
3. **Test Filters** - Apply various filter types
4. **Test Delay Effect** - Echo and delay processing
5. **Test Reverb Effect** - Spatial reverb simulation
6. **Full Synth Demo** - Complete signal chain demonstration
7. **Show Engine Status** - Real-time metrics and monitoring

## Architecture

The audio engine is built on a modular processor architecture:

```
AudioEngine
    ├── Device Management (PortAudio)
    ├── Transport Control (Start/Stop/Pause)
    ├── Audio Graph (Processor Chain)
    │   ├── Oscillators (Sound Generation)
    │   ├── Filters (Frequency Shaping)
    │   ├── Effects (Delay, Reverb, etc.)
    │   └── Custom Processors (Extensible)
    └── Metering & Monitoring
```

### Audio Processing Flow

1. **Input** → Audio device or generators
2. **Processing** → Through processor chain
3. **Mixing** → Master volume and combining
4. **Output** → Audio device with metering

## API Example

```cpp
#include "AudioEngine.h"
#include "Oscillator.h"
#include "Filter.h"

// Initialize engine
omega::AudioEngine engine;
engine.initialize(48000, 256, 2);

// Create processors
auto oscillator = std::make_shared<omega::Oscillator>(
    omega::WaveformType::Saw, 440.0f
);
auto filter = std::make_shared<omega::BiquadFilter>(
    omega::FilterType::LowPass
);
filter->setFrequency(1000.0f);

// Build audio graph
engine.addProcessor(oscillator);
engine.addProcessor(filter);

// Start playback
engine.startPlayback();
```

## Performance

- **Latency**: ~5-12ms @ 48kHz / 256 samples
- **CPU Usage**: <5% for basic synthesis on modern CPUs
- **Thread-Safe**: Real-time audio thread isolated from UI

## License

MIT License - See LICENSE file for details

## Contributing

Contributions are welcome! Please read CONTRIBUTING.md for guidelines.

## Roadmap

### Phase 1: Audio Engine (Current)
- [x] Real-time audio I/O
- [x] Basic DSP components
- [x] Audio graph architecture
- [x] MIDI input/output
- [x] Audio file I/O (WAV, MP3, FLAC)

### Phase 2: DAW Features
- [ ] Multi-track recording
- [ ] Timeline and arrangement
- [ ] Plugin hosting (VST3)
- [ ] Mixing console

### Phase 3: UI Development
- [ ] Modern GUI framework
- [ ] Waveform display
- [ ] Piano roll editor
- [ ] Mixer interface

### Phase 4: Advanced Features
- [ ] Automation lanes
- [ ] Side-chain processing
- [ ] Advanced routing
- [ ] Project templates

## Contact

Project maintained by omegadevine
GitHub: https://github.com/omegadevine/omega-digital-audio-workshop
