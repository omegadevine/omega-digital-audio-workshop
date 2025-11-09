#include "AudioEngine.h"
#include "AudioBuffer.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <string>
#include <sstream>

// Define M_PI for Windows
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace OmegaDAW;

// Simple tone generator processor for testing
class ToneGenerator : public IAudioProcessor {
public:
    ToneGenerator(float frequency = 440.0f, float amplitude = 0.3f) 
        : frequency_(frequency)
        , amplitude_(amplitude)
        , phase_(0.0)
        , sampleRate_(48000) {
    }
    
    void prepare(int sampleRate, int maxBufferSize) override {
        sampleRate_ = sampleRate;
        phase_ = 0.0;
    }
    
    void process(float** inputs, float** outputs, int numChannels, int numFrames) override {
        if (bypassed_) {
            // Copy input to output when bypassed
            for (int ch = 0; ch < numChannels; ++ch) {
                for (int i = 0; i < numFrames; ++i) {
                    outputs[ch][i] = 0.0f;
                }
            }
            return;
        }
        
        double phaseIncrement = 2.0 * M_PI * frequency_ / sampleRate_;
        
        for (int i = 0; i < numFrames; ++i) {
            float sample = amplitude_ * std::sin(phase_);
            phase_ += phaseIncrement;
            if (phase_ > 2.0 * M_PI) {
                phase_ -= 2.0 * M_PI;
            }
            
            // Write to all channels
            for (int ch = 0; ch < numChannels; ++ch) {
                outputs[ch][i] = sample;
            }
        }
    }
    
    std::string getName() const override {
        std::ostringstream oss;
        oss << "Tone Generator (" << static_cast<int>(frequency_) << " Hz)";
        return oss.str();
    }
    
    void setFrequency(float freq) { frequency_ = freq; }
    void setAmplitude(float amp) { amplitude_ = amp; }
    
private:
    float frequency_;
    float amplitude_;
    double phase_;
    int sampleRate_;
};

// Multi-tone chord generator
class ChordGenerator : public IAudioProcessor {
public:
    ChordGenerator(const std::vector<float>& frequencies, float amplitude = 0.2f)
        : frequencies_(frequencies)
        , amplitude_(amplitude)
        , sampleRate_(48000) {
        phases_.resize(frequencies.size(), 0.0);
    }
    
    void prepare(int sampleRate, int maxBufferSize) override {
        sampleRate_ = sampleRate;
        for (auto& phase : phases_) {
            phase = 0.0;
        }
    }
    
    void process(float** inputs, float** outputs, int numChannels, int numFrames) override {
        if (bypassed_) {
            for (int ch = 0; ch < numChannels; ++ch) {
                for (int i = 0; i < numFrames; ++i) {
                    outputs[ch][i] = 0.0f;
                }
            }
            return;
        }
        
        for (int i = 0; i < numFrames; ++i) {
            float sample = 0.0f;
            
            // Sum all frequencies
            for (size_t f = 0; f < frequencies_.size(); ++f) {
                double phaseIncrement = 2.0 * M_PI * frequencies_[f] / sampleRate_;
                sample += amplitude_ * std::sin(phases_[f]);
                phases_[f] += phaseIncrement;
                if (phases_[f] > 2.0 * M_PI) {
                    phases_[f] -= 2.0 * M_PI;
                }
            }
            
            // Normalize by number of tones
            sample /= frequencies_.size();
            
            // Write to all channels
            for (int ch = 0; ch < numChannels; ++ch) {
                outputs[ch][i] = sample;
            }
        }
    }
    
    std::string getName() const override {
        return "Chord Generator";
    }
    
private:
    std::vector<float> frequencies_;
    std::vector<double> phases_;
    float amplitude_;
    int sampleRate_;
};

void printDevices(AudioEngine& engine) {
    std::cout << "\n=== Available Audio Devices ===" << std::endl;
    auto devices = engine.getAvailableDevices();
    for (const auto& dev : devices) {
        std::cout << "Device #" << dev.index << ": " << dev.name << std::endl;
        std::cout << "  Input channels: " << dev.maxInputChannels << std::endl;
        std::cout << "  Output channels: " << dev.maxOutputChannels << std::endl;
        std::cout << "  Sample rate: " << dev.defaultSampleRate << " Hz" << std::endl;
    }
    std::cout << std::endl;
}

void printEngineInfo(AudioEngine& engine) {
    std::cout << "\n=== Audio Engine Status ===" << std::endl;
    std::cout << "Sample Rate: " << engine.getSampleRate() << " Hz" << std::endl;
    std::cout << "Buffer Size: " << engine.getBufferSize() << " samples" << std::endl;
    std::cout << "Channels: " << engine.getNumChannels() << std::endl;
    std::cout << "CPU Load: " << std::fixed << std::setprecision(1) 
              << engine.getCPULoad() << "%" << std::endl;
    std::cout << "Output Latency: " << std::fixed << std::setprecision(2) 
              << engine.getOutputLatency() * 1000.0 << " ms" << std::endl;
    std::cout << "Playing: " << (engine.isPlaying() ? "Yes" : "No") << std::endl;
    std::cout << "Master Volume: " << std::fixed << std::setprecision(2) 
              << engine.getMasterVolume() << std::endl;
    std::cout << std::endl;
}

void printMeterLevels(AudioEngine& engine) {
    int numChannels = engine.getNumChannels();
    std::cout << "Levels: ";
    for (int ch = 0; ch < numChannels; ++ch) {
        float peak = engine.getPeakLevel(ch);
        float rms = engine.getRMSLevel(ch);
        std::cout << "CH" << ch << " [Peak: " << std::fixed << std::setprecision(2) 
                  << peak << " RMS: " << rms << "] ";
    }
    std::cout << "\r" << std::flush;
}

void runTest1_SimpleTone(AudioEngine& engine) {
    std::cout << "\n### TEST 1: Simple 440Hz Tone ###" << std::endl;
    
    auto toneGen = std::make_shared<ToneGenerator>(440.0f, 0.3f);
    engine.addProcessor(toneGen);
    
    std::cout << "Playing 440Hz tone for 3 seconds..." << std::endl;
    engine.startPlayback();
    
    for (int i = 0; i < 30; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        printMeterLevels(engine);
    }
    
    engine.stopPlayback();
    engine.clearProcessors();
    std::cout << "\nTest 1 complete." << std::endl;
}

void runTest2_VolumeControl(AudioEngine& engine) {
    std::cout << "\n### TEST 2: Volume Control ###" << std::endl;
    
    auto toneGen = std::make_shared<ToneGenerator>(440.0f, 0.5f);
    engine.addProcessor(toneGen);
    engine.startPlayback();
    
    std::cout << "Ramping volume from 0.0 to 1.0 over 3 seconds..." << std::endl;
    for (int i = 0; i <= 30; ++i) {
        float volume = i / 30.0f;
        engine.setMasterVolume(volume);
        std::cout << "Volume: " << std::fixed << std::setprecision(2) << volume << " ";
        printMeterLevels(engine);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "\nRamping volume from 1.0 to 0.0 over 3 seconds..." << std::endl;
    for (int i = 30; i >= 0; --i) {
        float volume = i / 30.0f;
        engine.setMasterVolume(volume);
        std::cout << "Volume: " << std::fixed << std::setprecision(2) << volume << " ";
        printMeterLevels(engine);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    engine.stopPlayback();
    engine.clearProcessors();
    engine.setMasterVolume(1.0f);
    std::cout << "\nTest 2 complete." << std::endl;
}

void runTest3_MultipleFrequencies(AudioEngine& engine) {
    std::cout << "\n### TEST 3: Multiple Frequencies (Scale) ###" << std::endl;
    
    // C major scale frequencies
    std::vector<float> scale = {
        261.63f,  // C4
        293.66f,  // D4
        329.63f,  // E4
        349.23f,  // F4
        392.00f,  // G4
        440.00f,  // A4
        493.88f,  // B4
        523.25f   // C5
    };
    
    std::cout << "Playing C major scale..." << std::endl;
    for (size_t i = 0; i < scale.size(); ++i) {
        auto toneGen = std::make_shared<ToneGenerator>(scale[i], 0.3f);
        engine.addProcessor(toneGen);
        engine.startPlayback();
        
        std::cout << "Note " << (i + 1) << " (" << (int)scale[i] << " Hz) ";
        for (int j = 0; j < 5; ++j) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            printMeterLevels(engine);
        }
        std::cout << std::endl;
        
        engine.stopPlayback();
        engine.clearProcessors();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    std::cout << "Test 3 complete." << std::endl;
}

void runTest4_ChordPlayback(AudioEngine& engine) {
    std::cout << "\n### TEST 4: Chord Playback ###" << std::endl;
    
    // C major chord (C-E-G)
    std::vector<float> cMajor = {261.63f, 329.63f, 392.00f};
    auto chordGen = std::make_shared<ChordGenerator>(cMajor, 0.25f);
    engine.addProcessor(chordGen);
    
    std::cout << "Playing C major chord for 3 seconds..." << std::endl;
    engine.startPlayback();
    
    for (int i = 0; i < 30; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        printMeterLevels(engine);
    }
    
    engine.stopPlayback();
    engine.clearProcessors();
    std::cout << "\nTest 4 complete." << std::endl;
}

void runTest5_StartStopTiming(AudioEngine& engine) {
    std::cout << "\n### TEST 5: Start/Stop Timing ###" << std::endl;
    
    auto toneGen = std::make_shared<ToneGenerator>(440.0f, 0.3f);
    engine.addProcessor(toneGen);
    
    std::cout << "Rapid start/stop test (10 iterations)..." << std::endl;
    for (int i = 0; i < 10; ++i) {
        std::cout << "Iteration " << (i + 1) << ": ";
        
        auto start = std::chrono::high_resolution_clock::now();
        engine.startPlayback();
        auto startDone = std::chrono::high_resolution_clock::now();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        auto stop = std::chrono::high_resolution_clock::now();
        engine.stopPlayback();
        auto stopDone = std::chrono::high_resolution_clock::now();
        
        auto startLatency = std::chrono::duration_cast<std::chrono::microseconds>(startDone - start);
        auto stopLatency = std::chrono::duration_cast<std::chrono::microseconds>(stopDone - stop);
        
        std::cout << "Start: " << startLatency.count() << "μs, "
                  << "Stop: " << stopLatency.count() << "μs" << std::endl;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    engine.clearProcessors();
    std::cout << "Test 5 complete." << std::endl;
}

void runTest6_CPULoadMonitoring(AudioEngine& engine) {
    std::cout << "\n### TEST 6: CPU Load Monitoring ###" << std::endl;
    
    // Add multiple processors to increase load
    for (int i = 0; i < 5; ++i) {
        auto toneGen = std::make_shared<ToneGenerator>(440.0f + i * 100, 0.1f);
        engine.addProcessor(toneGen);
    }
    
    std::cout << "Monitoring CPU load with 5 tone generators..." << std::endl;
    engine.startPlayback();
    
    for (int i = 0; i < 30; ++i) {
        std::cout << "CPU: " << std::fixed << std::setprecision(1) 
                  << engine.getCPULoad() << "% | ";
        printMeterLevels(engine);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    engine.stopPlayback();
    engine.clearProcessors();
    std::cout << "\nTest 6 complete." << std::endl;
}

void runTest7_ProcessorBypass(AudioEngine& engine) {
    std::cout << "\n### TEST 7: Processor Bypass ###" << std::endl;
    
    auto toneGen = std::make_shared<ToneGenerator>(440.0f, 0.3f);
    engine.addProcessor(toneGen);
    engine.startPlayback();
    
    std::cout << "Playing tone with periodic bypass (10 iterations)..." << std::endl;
    for (int i = 0; i < 10; ++i) {
        bool bypassed = (i % 2 == 0);
        toneGen->setBypassed(bypassed);
        std::cout << (bypassed ? "BYPASSED " : "ACTIVE   ") << "| ";
        
        for (int j = 0; j < 5; ++j) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            printMeterLevels(engine);
        }
        std::cout << std::endl;
    }
    
    engine.stopPlayback();
    engine.clearProcessors();
    std::cout << "Test 7 complete." << std::endl;
}

int main() {
    std::cout << "==================================" << std::endl;
    std::cout << "  Omega DAW Audio Playback Test  " << std::endl;
    std::cout << "==================================" << std::endl;
    
    AudioEngine engine;
    
    // List available devices
    printDevices(engine);
    
    // Initialize audio engine
    std::cout << "Initializing audio engine..." << std::endl;
    if (!engine.initialize(48000, 256, 2)) {
        std::cerr << "Failed to initialize audio engine!" << std::endl;
        return 1;
    }
    
    printEngineInfo(engine);
    
    // Run all tests
    try {
        runTest1_SimpleTone(engine);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        runTest2_VolumeControl(engine);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        runTest3_MultipleFrequencies(engine);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        runTest4_ChordPlayback(engine);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        runTest5_StartStopTiming(engine);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        runTest6_CPULoadMonitoring(engine);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        runTest7_ProcessorBypass(engine);
        
    } catch (const std::exception& e) {
        std::cerr << "Test error: " << e.what() << std::endl;
        engine.shutdown();
        return 1;
    }
    
    // Cleanup
    std::cout << "\n==================================" << std::endl;
    std::cout << "All tests completed successfully!" << std::endl;
    std::cout << "==================================" << std::endl;
    
    engine.shutdown();
    return 0;
}
