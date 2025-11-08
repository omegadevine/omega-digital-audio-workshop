# Omega Digital Audio Workshop

A professional Digital Audio Workstation (DAW) built from the ground up.

## Features (Planned)

- Multi-track audio recording and playback
- MIDI sequencing and editing
- VST/VST3 plugin support
- Built-in synthesizers and effects
- Non-destructive editing
- Audio mixing with EQ, compression, and effects
- Automation support
- Real-time audio processing
- Project management and session handling

## System Requirements

- Windows 10/11 (64-bit)
- 4GB RAM minimum (8GB+ recommended)
- Audio interface compatible with ASIO/WASAPI

## Building from Source

### Prerequisites
- CMake 3.15 or higher
- C++17 compatible compiler (MSVC, GCC, or Clang)
- Git

### Dependencies
- JUCE Framework (for audio processing and UI)
- PortAudio (cross-platform audio I/O)
- RtMidi (for MIDI support)
- SQLite (for project data)

### Build Instructions
```bash
git clone https://github.com/omegadevine/omega-digital-audio-workshop.git
cd omega-digital-audio-workshop
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Getting Started

Documentation coming soon.

## License

MIT License - See LICENSE file for details

## Contributing

Contributions are welcome! Please read CONTRIBUTING.md for guidelines.

## Contact

Project maintained by omegadevine
