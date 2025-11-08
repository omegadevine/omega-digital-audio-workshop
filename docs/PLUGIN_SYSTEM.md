# Plugin System Documentation

## Overview
The Omega DAW Plugin System provides a flexible architecture for audio processing plugins with support for effects, instruments, analyzers, and utilities.

## Architecture

### Core Components

#### Plugin Base Class
- Abstract base class for all plugins
- Handles parameter management
- Supports bypass and enable/disable states
- Process audio in plugin chain

#### Plugin Host
- Manages plugin chain
- Routes audio through plugins
- Handles plugin ordering and removal
- Manages intermediate buffers

### Plugin Types
1. **Effect** - Audio processing (reverb, delay, EQ)
2. **Instrument** - Sound generation (synthesizers, samplers)
3. **Analyzer** - Audio analysis (spectrum, meters)
4. **Utility** - Signal routing and utility functions

## Built-in Plugins

### 1. Gain Plugin
Simple gain control with volume adjustment.

**Parameters:**
- `gain` (0.0 - 2.0): Volume multiplier

### 2. Delay Plugin
Time-based delay effect with feedback control.

**Parameters:**
- `delaytime` (0.0 - 2.0s): Delay time
- `feedback` (0.0 - 0.95): Feedback amount
- `mix` (0.0 - 1.0): Dry/wet mix

### 3. Reverb Plugin
Algorithmic reverb using comb and allpass filters.

**Parameters:**
- `roomsize` (0.0 - 1.0): Room size simulation
- `damping` (0.0 - 1.0): High frequency damping
- `mix` (0.0 - 1.0): Dry/wet mix

### 4. Compressor Plugin
Dynamic range compressor with envelope follower.

**Parameters:**
- `threshold` (-60.0 - 0.0 dB): Compression threshold
- `ratio` (1.0 - 20.0:1): Compression ratio
- `attack` (0.001 - 0.1s): Attack time
- `release` (0.01 - 1.0s): Release time

### 5. EQ Plugin
Three-band parametric equalizer.

**Parameters:**
- `low_gain` (-12.0 - 12.0 dB): Low band gain
- `low_freq` (20.0 - 20000.0 Hz): Low band frequency
- `mid_gain` (-12.0 - 12.0 dB): Mid band gain
- `mid_freq` (20.0 - 20000.0 Hz): Mid band frequency
- `high_gain` (-12.0 - 12.0 dB): High band gain
- `high_freq` (20.0 - 20000.0 Hz): High band frequency

## Usage Example

```cpp
#include "PluginHost.h"
#include "BuiltInPlugins.h"

// Create plugin host
PluginHost host;
host.initialize(44100, 512);

// Add plugins
auto eq = std::make_shared<EQPlugin>();
host.addPlugin(eq);

auto compressor = std::make_shared<CompressorPlugin>();
host.addPlugin(compressor);

auto reverb = std::make_shared<ReverbPlugin>();
host.addPlugin(reverb);

// Set parameters
eq->setParameter("mid_freq", 1000.0f);
eq->setParameter("mid_gain", 3.0f);

compressor->setParameter("threshold", -15.0f);
compressor->setParameter("ratio", 4.0f);

// Process audio
host.processPluginChain(inputs, outputs, 2, 512);
```

## Creating Custom Plugins

### Step 1: Inherit from Plugin

```cpp
class MyCustomPlugin : public Plugin {
public:
    MyCustomPlugin() : Plugin("MyPlugin", PluginType::Effect) {
        // Add parameters
        PluginParameter param;
        param.id = "my_param";
        param.name = "My Parameter";
        param.value = 0.5f;
        param.minValue = 0.0f;
        param.maxValue = 1.0f;
        param.defaultValue = 0.5f;
        addParameter(param);
    }
    
    void initialize(int sampleRate, int maxBufferSize) override {
        this->sampleRate = sampleRate;
        this->maxBufferSize = maxBufferSize;
        // Initialize buffers, filters, etc.
    }
    
    void process(float** inputs, float** outputs, 
                 int numChannels, int numSamples) override {
        float myParam = getParameter("my_param");
        
        for (int ch = 0; ch < numChannels; ++ch) {
            for (int i = 0; i < numSamples; ++i) {
                outputs[ch][i] = inputs[ch][i] * myParam;
            }
        }
    }
    
    void reset() override {
        // Reset internal state
    }
};
```

## Parameter Automation

All parameters with `isAutomatable = true` can be automated:

```cpp
// Get plugin from host
auto plugin = host.getPlugin(0);

// Automate parameter over time
for (int i = 0; i < numSamples; ++i) {
    float automationValue = getAutomationValue(i);
    plugin->setParameter("gain", automationValue);
}
```

## Plugin Chain Management

```cpp
// Add plugin
host.addPlugin(myPlugin);

// Remove plugin
host.removePlugin(1);

// Move plugin in chain
host.movePlugin(2, 0); // Move plugin from index 2 to 0

// Bypass plugin
host.setPluginBypass(0, true);

// Enable/disable plugin
host.setPluginEnabled(1, false);

// Clear all plugins
host.clearPlugins();

// Reset all plugins
host.resetAllPlugins();
```

## Performance Considerations

1. **Buffer Management**: Plugin host manages intermediate buffers automatically
2. **Bypass**: Bypassed plugins copy input to output without processing
3. **Enable/Disable**: Disabled plugins are skipped entirely
4. **Parameter Updates**: Parameter changes are thread-safe
5. **Memory Allocation**: Pre-allocate buffers in initialize() method

## Future Extensions

### VST3 Support
- Load external VST3 plugins
- Map VST3 parameters to plugin interface
- Handle VST3 events and automation

### Plugin Presets
- Save/load plugin parameter presets
- Preset management system
- Factory presets for built-in plugins

### Plugin GUI
- Visual plugin interface
- Parameter controls and displays
- Automation visualization

### Advanced Features
- Sidechain support
- MIDI input for plugins
- Multi-output plugins
- Plugin sandboxing and safety
