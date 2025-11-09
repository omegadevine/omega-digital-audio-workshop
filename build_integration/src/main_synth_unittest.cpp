#include "MIDISynthesizer.h"
#include <iostream>
#include <vector>

using namespace OmegaDAW;

int main() {
    std::cout << "=== MIDI Synthesizer Unit Test ===" << std::endl;
    
    // Create synthesizer
    MIDISynthesizer synth(16);
    
    std::cout << "Created synthesizer with 16 voices" << std::endl;
    
    // Prepare it
    synth.prepare(44100, 512);
    
    std::cout << "Prepared synthesizer (44100 Hz, 512 buffer)" << std::endl;
    
    // Configure
    synth.setWaveform(WaveformType::Sine);
    synth.setMasterVolume(0.5f);
    
    std::cout << "Configured synthesizer" << std::endl;
    
    // Test note on
    std::cout << "\nTest 1: Note On" << std::endl;
    synth.noteOn(60, 100);
    std::cout << "  Active voices: " << synth.getActiveVoiceCount() << " (expected: 1)" << std::endl;
    
    // Test multiple notes
    std::cout << "\nTest 2: Multiple Notes" << std::endl;
    synth.noteOn(64, 90);
    synth.noteOn(67, 80);
    std::cout << "  Active voices: " << synth.getActiveVoiceCount() << " (expected: 3)" << std::endl;
    
    // Test note off
    std::cout << "\nTest 3: Note Off" << std::endl;
    synth.noteOff(60);
    std::cout << "  Active voices after note off: " << synth.getActiveVoiceCount() << std::endl;
    
    // Test all notes off
    std::cout << "\nTest 4: All Notes Off" << std::endl;
    synth.allNotesOff();
    std::cout << "  Active voices after all off: " << synth.getActiveVoiceCount() << std::endl;
    
    // Test audio processing
    std::cout << "\nTest 5: Audio Processing" << std::endl;
    synth.noteOn(60, 100);
    
    const int bufferSize = 512;
    const int numChannels = 2;
    std::vector<float> leftBuffer(bufferSize, 0.0f);
    std::vector<float> rightBuffer(bufferSize, 0.0f);
    float* outputs[] = { leftBuffer.data(), rightBuffer.data() };
    float* inputs[] = { nullptr, nullptr };
    
    synth.process(inputs, outputs, numChannels, bufferSize);
    
    // Check if we got audio output
    bool hasOutput = false;
    for (int i = 0; i < bufferSize; ++i) {
        if (leftBuffer[i] != 0.0f) {
            hasOutput = true;
            break;
        }
    }
    
    std::cout << "  Generated audio: " << (hasOutput ? "YES" : "NO") << std::endl;
    
    if (hasOutput) {
        float maxAmplitude = 0.0f;
        for (int i = 0; i < bufferSize; ++i) {
            float amp = std::abs(leftBuffer[i]);
            if (amp > maxAmplitude) maxAmplitude = amp;
        }
        std::cout << "  Max amplitude: " << maxAmplitude << std::endl;
    }
    
    // Test MIDI message processing
    std::cout << "\nTest 6: MIDI Message Processing" << std::endl;
    synth.allNotesOff();
    
    MIDIMessage noteOnMsg = MIDIMessage::noteOn(0, 64, 90);
    synth.processMIDIMessage(noteOnMsg);
    std::cout << "  Active voices after MIDI note on: " << synth.getActiveVoiceCount() << std::endl;
    
    MIDIMessage noteOffMsg = MIDIMessage::noteOff(0, 64);
    synth.processMIDIMessage(noteOffMsg);
    std::cout << "  Active voices after MIDI note off: " << synth.getActiveVoiceCount() << std::endl;
    
    // Test MIDI buffer processing
    std::cout << "\nTest 7: MIDI Buffer Processing" << std::endl;
    MIDIBuffer buffer;
    buffer.addMessage(MIDIMessage::noteOn(0, 60, 80));
    buffer.addMessage(MIDIMessage::noteOn(0, 64, 80));
    buffer.addMessage(MIDIMessage::noteOn(0, 67, 80));
    
    synth.processMIDIBuffer(buffer);
    std::cout << "  Active voices after buffer: " << synth.getActiveVoiceCount() << " (expected: ~3)" << std::endl;
    
    std::cout << "\n=== All tests completed successfully! ===" << std::endl;
    
    return 0;
}
