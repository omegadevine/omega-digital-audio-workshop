#include "AudioEngine.h"
#include "Oscillator.h"
#include "Filter.h"
#include "Effects.h"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

void printMenu() {
    std::cout << "\n================================" << std::endl;
    std::cout << "Omega DAW - Audio Engine Demo" << std::endl;
    std::cout << "================================" << std::endl;
    std::cout << "1. List Audio Devices" << std::endl;
    std::cout << "2. Test Oscillators" << std::endl;
    std::cout << "3. Test Filters" << std::endl;
    std::cout << "4. Test Delay Effect" << std::endl;
    std::cout << "5. Test Reverb Effect" << std::endl;
    std::cout << "6. Full Synth Demo (Oscillator + Filter + Effects)" << std::endl;
    std::cout << "7. Show Engine Status" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "\nChoice: ";
}

void listDevices(omega::AudioEngine& engine) {
    auto devices = engine.getAvailableDevices();
    std::cout << "\n=== Available Audio Devices ===" << std::endl;
    
    for (const auto& device : devices) {
        std::cout << "[" << device.index << "] " << device.name << std::endl;
        std::cout << "    Inputs: " << device.maxInputChannels 
                  << ", Outputs: " << device.maxOutputChannels << std::endl;
        std::cout << "    Default Sample Rate: " << device.defaultSampleRate << " Hz" << std::endl;
    }
}

void testOscillators(omega::AudioEngine& engine) {
    std::cout << "\n=== Testing Oscillators ===" << std::endl;
    std::cout << "Playing: Sine (440Hz) -> Square (220Hz) -> Saw (330Hz)" << std::endl;
    
    // Create oscillators
    auto sine = std::make_shared<omega::Oscillator>(omega::WaveformType::Sine, 440.0f);
    sine->setAmplitude(0.2f);
    
    engine.clearProcessors();
    engine.addProcessor(sine);
    engine.startPlayback();
    
    std::cout << "Sine wave (2 sec)..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Switch to square
    auto square = std::make_shared<omega::Oscillator>(omega::WaveformType::Square, 220.0f);
    square->setAmplitude(0.15f);
    engine.clearProcessors();
    engine.addProcessor(square);
    
    std::cout << "Square wave (2 sec)..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Switch to saw
    auto saw = std::make_shared<omega::Oscillator>(omega::WaveformType::Saw, 330.0f);
    saw->setAmplitude(0.15f);
    engine.clearProcessors();
    engine.addProcessor(saw);
    
    std::cout << "Saw wave (2 sec)..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    engine.stopPlayback();
    std::cout << "Test complete!" << std::endl;
}

void testFilters(omega::AudioEngine& engine) {
    std::cout << "\n=== Testing Filters ===" << std::endl;
    std::cout << "Playing: Saw wave through LowPass -> HighPass -> BandPass filters" << std::endl;
    
    auto saw = std::make_shared<omega::Oscillator>(omega::WaveformType::Saw, 220.0f);
    saw->setAmplitude(0.2f);
    
    auto filter = std::make_shared<omega::BiquadFilter>(omega::FilterType::LowPass);
    filter->setFrequency(800.0f);
    filter->setQ(2.0f);
    
    engine.clearProcessors();
    engine.addProcessor(saw);
    engine.addProcessor(filter);
    engine.startPlayback();
    
    std::cout << "LowPass @ 800Hz (3 sec)..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    filter->setType(omega::FilterType::HighPass);
    filter->setFrequency(400.0f);
    std::cout << "HighPass @ 400Hz (3 sec)..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    filter->setType(omega::FilterType::BandPass);
    filter->setFrequency(600.0f);
    std::cout << "BandPass @ 600Hz (3 sec)..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    engine.stopPlayback();
    std::cout << "Test complete!" << std::endl;
}

void testDelay(omega::AudioEngine& engine) {
    std::cout << "\n=== Testing Delay Effect ===" << std::endl;
    std::cout << "Playing: Triangle wave with delay" << std::endl;
    
    auto triangle = std::make_shared<omega::Oscillator>(omega::WaveformType::Triangle, 330.0f);
    triangle->setAmplitude(0.25f);
    
    auto delay = std::make_shared<omega::Delay>(300.0f, 0.6f, 0.5f);
    
    engine.clearProcessors();
    engine.addProcessor(triangle);
    engine.addProcessor(delay);
    engine.startPlayback();
    
    std::cout << "Delay: 300ms, feedback: 0.6, mix: 0.5 (5 sec)..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    engine.stopPlayback();
    std::cout << "Test complete!" << std::endl;
}

void testReverb(omega::AudioEngine& engine) {
    std::cout << "\n=== Testing Reverb Effect ===" << std::endl;
    std::cout << "Playing: Sine wave with reverb" << std::endl;
    
    auto sine = std::make_shared<omega::Oscillator>(omega::WaveformType::Sine, 440.0f);
    sine->setAmplitude(0.2f);
    
    auto reverb = std::make_shared<omega::Reverb>(0.7f, 0.5f, 0.4f);
    
    engine.clearProcessors();
    engine.addProcessor(sine);
    engine.addProcessor(reverb);
    engine.startPlayback();
    
    std::cout << "Room size: 0.7, damping: 0.5, mix: 0.4 (5 sec)..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    engine.stopPlayback();
    std::cout << "Test complete!" << std::endl;
}

void fullSynthDemo(omega::AudioEngine& engine) {
    std::cout << "\n=== Full Synth Demo ===" << std::endl;
    std::cout << "Playing: Saw wave -> Filter -> Delay -> Reverb" << std::endl;
    
    // Create audio chain
    auto saw = std::make_shared<omega::Oscillator>(omega::WaveformType::Saw, 220.0f);
    saw->setAmplitude(0.2f);
    
    auto filter = std::make_shared<omega::BiquadFilter>(omega::FilterType::LowPass);
    filter->setFrequency(1200.0f);
    filter->setQ(3.0f);
    
    auto delay = std::make_shared<omega::Delay>(375.0f, 0.4f, 0.3f);
    auto reverb = std::make_shared<omega::Reverb>(0.6f, 0.4f, 0.25f);
    
    engine.clearProcessors();
    engine.addProcessor(saw);
    engine.addProcessor(filter);
    engine.addProcessor(delay);
    engine.addProcessor(reverb);
    engine.startPlayback();
    
    std::cout << "Playing for 8 seconds..." << std::endl;
    
    // Animate filter cutoff
    for (int i = 0; i < 8; ++i) {
        float t = i / 8.0f;
        float freq = 400.0f + (1600.0f * t);
        filter->setFrequency(freq);
        std::cout << "Filter cutoff: " << freq << " Hz" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    engine.stopPlayback();
    std::cout << "Demo complete!" << std::endl;
}

void showStatus(omega::AudioEngine& engine) {
    std::cout << "\n=== Engine Status ===" << std::endl;
    std::cout << "Initialized: " << (engine.isInitialized() ? "Yes" : "No") << std::endl;
    std::cout << "Playing: " << (engine.isPlaying() ? "Yes" : "No") << std::endl;
    std::cout << "Sample Rate: " << engine.getSampleRate() << " Hz" << std::endl;
    std::cout << "Buffer Size: " << engine.getBufferSize() << " samples" << std::endl;
    std::cout << "Channels: " << engine.getNumChannels() << std::endl;
    std::cout << "CPU Load: " << (engine.getCPULoad() * 100.0f) << "%" << std::endl;
    std::cout << "Current Time: " << engine.getCurrentTime() << " sec" << std::endl;
    std::cout << "Master Volume: " << engine.getMasterVolume() << std::endl;
    std::cout << "Output Latency: " << (engine.getOutputLatency() * 1000.0) << " ms" << std::endl;
    
    std::cout << "\nMetering:" << std::endl;
    for (int ch = 0; ch < engine.getNumChannels(); ++ch) {
        std::cout << "  Channel " << ch << " - Peak: " << engine.getPeakLevel(ch) 
                  << ", RMS: " << engine.getRMSLevel(ch) << std::endl;
    }
}

int main() {
    std::cout << "==================================" << std::endl;
    std::cout << " Omega Digital Audio Workshop" << std::endl;
    std::cout << " Version 0.1.0" << std::endl;
    std::cout << "==================================" << std::endl;
    
    omega::AudioEngine engine;
    
    // Initialize with professional settings
    if (!engine.initialize(48000, 256, 2)) {
        std::cerr << "Failed to initialize audio engine" << std::endl;
        std::cout << "\nPress Enter to exit..." << std::endl;
        std::cin.get();
        return 1;
    }
    
    std::cout << "\nAudio engine initialized successfully!" << std::endl;
    
    bool running = true;
    while (running) {
        printMenu();
        
        int choice;
        std::cin >> choice;
        std::cin.ignore();
        
        switch (choice) {
            case 1:
                listDevices(engine);
                break;
            case 2:
                testOscillators(engine);
                break;
            case 3:
                testFilters(engine);
                break;
            case 4:
                testDelay(engine);
                break;
            case 5:
                testReverb(engine);
                break;
            case 6:
                fullSynthDemo(engine);
                break;
            case 7:
                showStatus(engine);
                break;
            case 0:
                running = false;
                break;
            default:
                std::cout << "Invalid choice!" << std::endl;
        }
    }
    
    engine.shutdown();
    
    std::cout << "\nGoodbye!" << std::endl;
    
    return 0;
}
