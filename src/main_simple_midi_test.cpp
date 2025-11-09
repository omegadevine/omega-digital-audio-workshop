#include "AudioEngine.h"
#include "MIDISynthesizer.h"
#include "MIDISequencer.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace OmegaDAW;

int main(int argc, char* argv[]) {
    std::cout << "=== Simple MIDI Synthesizer Test ===" << std::endl;
    
    // Create audio engine
    AudioEngine audioEngine;
    
    if (!audioEngine.initialize(44100, 512, 2)) {
        std::cerr << "Failed to initialize audio engine" << std::endl;
        return 1;
    }
    
    std::cout << "Audio engine initialized" << std::endl;
    std::cout << "Sample rate: " << audioEngine.getSampleRate() << " Hz" << std::endl;
    std::cout << "Buffer size: " << audioEngine.getBufferSize() << " samples" << std::endl;
    
    // Create and configure synthesizer
    auto synth = std::make_shared<MIDISynthesizer>(16);
    synth->prepare(audioEngine.getSampleRate(), audioEngine.getBufferSize());
    synth->setWaveform(WaveformType::Sine);
    synth->setAttack(0.05f);
    synth->setRelease(0.2f);
    synth->setMasterVolume(0.2f);
    
    // Add synthesizer to audio engine
    audioEngine.addProcessor(synth);
    
    std::cout << "MIDI Synthesizer created and connected" << std::endl;
    
    // Start playback
    audioEngine.startPlayback();
    std::cout << "\n=== Playing MIDI notes ===" << std::endl;
    
    // Test 1: Play a single note
    std::cout << "Test 1: Playing middle C (MIDI 60)..." << std::endl;
    synth->noteOn(60, 100);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    synth->noteOff(60);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Test 2: Play a scale
    std::cout << "Test 2: Playing C major scale..." << std::endl;
    int scale[] = {60, 62, 64, 65, 67, 69, 71, 72}; // C major scale
    for (int note : scale) {
        synth->noteOn(note, 80);
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        synth->noteOff(note);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Test 3: Play a chord
    std::cout << "Test 3: Playing C major chord..." << std::endl;
    synth->noteOn(60, 80); // C
    synth->noteOn(64, 80); // E
    synth->noteOn(67, 80); // G
    std::this_thread::sleep_for(std::chrono::seconds(2));
    synth->allNotesOff();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Test 4: Different waveforms
    std::cout << "Test 4: Testing different waveforms..." << std::endl;
    
    synth->setWaveform(WaveformType::Square);
    std::cout << "  Square wave..." << std::endl;
    synth->noteOn(60, 70);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    synth->noteOff(60);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    synth->setWaveform(WaveformType::Saw);
    std::cout << "  Sawtooth wave..." << std::endl;
    synth->noteOn(60, 70);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    synth->noteOff(60);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    synth->setWaveform(WaveformType::Triangle);
    std::cout << "  Triangle wave..." << std::endl;
    synth->noteOn(60, 70);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    synth->noteOff(60);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Stop playback
    audioEngine.stopPlayback();
    
    std::cout << "\n=== Test complete ===" << std::endl;
    std::cout << "Shutting down..." << std::endl;
    
    audioEngine.shutdown();
    
    std::cout << "Done!" << std::endl;
    return 0;
}
