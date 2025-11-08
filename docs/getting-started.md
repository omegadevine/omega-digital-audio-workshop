# Getting Started with Omega DAW

## Installation

### Windows

1. Download the latest release from GitHub
2. Extract to desired location
3. Run `OmegaDAW.exe`

### Building from Source

#### Prerequisites
- CMake 3.15 or higher
- C++17 compatible compiler (MSVC 2019+, GCC 9+, Clang 10+)
- Git

#### Build Steps

```bash
# Clone the repository
git clone https://github.com/omegadevine/omega-digital-audio-workshop.git
cd omega-digital-audio-workshop

# Create build directory
mkdir build
cd build

# Configure
cmake ..

# Build
cmake --build . --config Release

# Run
./bin/OmegaDAW
```

## First Steps

### Creating a New Project

1. Launch Omega DAW
2. File → New Project
3. Set project name and location
4. Configure sample rate (44.1kHz or 48kHz recommended)
5. Set buffer size (512 samples is a good starting point)

### Adding Tracks

1. Right-click in track list
2. Select "Add Track"
3. Choose track type:
   - **Audio Track**: For recording and playing audio
   - **MIDI Track**: For MIDI sequencing
   - **Master Track**: Automatically created

### Recording Audio

1. Select an audio track
2. Enable record arm (red button)
3. Configure input source in preferences
4. Click record on transport
5. Play or sing into your microphone
6. Click stop when done

### Playing Audio

1. Import audio file: File → Import → Audio File
2. Drag to track or timeline
3. Click play on transport
4. Adjust volume and pan on track

### MIDI Recording

1. Select a MIDI track
2. Enable record arm
3. Connect MIDI keyboard/controller
4. Click record on transport
5. Play notes on your keyboard
6. Click stop when done

### Using Effects

1. Select a track
2. Click "Add Effect" in mixer
3. Choose effect type
4. Adjust parameters
5. Enable/bypass as needed

## Transport Controls

- **Play/Pause**: Space bar
- **Stop**: Enter
- **Record**: R
- **Rewind**: Home
- **Fast Forward**: End
- **Set Loop Start**: [
- **Set Loop End**: ]

## Keyboard Shortcuts

### File Operations
- `Ctrl+N`: New Project
- `Ctrl+O`: Open Project
- `Ctrl+S`: Save Project
- `Ctrl+Shift+S`: Save As

### Edit Operations
- `Ctrl+Z`: Undo
- `Ctrl+Y`: Redo
- `Ctrl+X`: Cut
- `Ctrl+C`: Copy
- `Ctrl+V`: Paste
- `Delete`: Delete Selection

### Track Operations
- `Ctrl+T`: Add Track
- `M`: Mute Selected Track
- `S`: Solo Selected Track
- `R`: Record Arm Selected Track

### View Operations
- `Ctrl+1`: Show Mixer
- `Ctrl+2`: Show Timeline
- `Ctrl+3`: Show Piano Roll
- `+`: Zoom In
- `-`: Zoom Out
- `0`: Zoom to Fit

## Audio Settings

### Sample Rate
- **44.1 kHz**: CD quality, most common
- **48 kHz**: Video standard
- **96 kHz**: High quality recording
- **192 kHz**: Ultra high quality (high CPU usage)

### Buffer Size
- **128 samples**: Low latency, high CPU usage
- **256 samples**: Balanced
- **512 samples**: Standard, good performance
- **1024 samples**: High latency, low CPU usage
- **2048 samples**: Mixing only, very high latency

Lower buffer size = lower latency but requires more CPU power.

## Tips for Best Performance

1. **Use ASIO drivers** on Windows for best latency
2. **Close unnecessary programs** while recording
3. **Disable WiFi** to reduce audio interference
4. **Use SSD** for project files and samples
5. **Freeze tracks** with heavy plugins to save CPU
6. **Increase buffer size** when mixing (not recording)
7. **Monitor CPU usage** and add more tracks gradually

## Troubleshooting

### Audio Not Playing
- Check audio device selection in preferences
- Verify driver is properly installed
- Try different buffer sizes
- Restart application

### High Latency
- Reduce buffer size in preferences
- Use ASIO driver (Windows)
- Close background applications
- Disable unnecessary effects

### Crackling/Popping
- Increase buffer size
- Reduce track count
- Freeze tracks with heavy plugins
- Check CPU usage

### Can't Record
- Verify input device is selected
- Check track is record-armed
- Verify microphone permissions (Mac/Linux)
- Test input levels in system settings

## Next Steps

- Read the [Architecture Documentation](architecture.md)
- Explore the [API Reference](api-reference.md)
- Check out example projects
- Join the community forum

## Support

- GitHub Issues: https://github.com/omegadevine/omega-digital-audio-workshop/issues
- Documentation: https://github.com/omegadevine/omega-digital-audio-workshop/docs
- Community: [Coming Soon]
