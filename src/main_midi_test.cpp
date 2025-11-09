#include "DAWApplication.h"
#include "MIDISynthesizer.h"
#include "MIDISequencer.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace OmegaDAW;

int main(int argc, char* argv[]) {
    std::cout << "=== Omega DAW MIDI Playback Test ===" << std::endl;
    
    // Create and initialize the DAW
    DAWApplication daw;
    
    if (!daw.initialize(44100, 512)) {
        std::cerr << "Failed to initialize DAW application" << std::endl;
        return 1;
    }
    
    std::cout << "DAW initialized successfully!" << std::endl;
    
    // Get components
    auto* midiSequencer = daw.getMIDISequencer();
    auto* midiSynth = daw.getMIDISynthesizer();
    auto* transport = daw.getTransport();
    auto* audioEngine = daw.getAudioEngine();
    
    // Configure synthesizer
    midiSynth->setWaveform(WaveformType::Sine);
    midiSynth->setAttack(0.01f);
    midiSynth->setDecay(0.1f);
    midiSynth->setSustain(0.7f);
    midiSynth->setRelease(0.3f);
    midiSynth->setMasterVolume(0.3f);
    
    // Create a simple MIDI pattern (C major scale)
    auto pattern = std::make_shared<MIDIPattern>();
    
    // C major scale: C, D, E, F, G, A, B, C
    int scaleNotes[] = {60, 62, 64, 65, 67, 69, 71, 72}; // MIDI note numbers
    
    std::cout << "Creating MIDI pattern (C major scale)..." << std::endl;
    
    for (int i = 0; i < 8; ++i) {
        double startTime = i * 0.5; // Half second per note
        double duration = 0.4;      // 400ms duration
        
        MIDINote note(0, scaleNotes[i], 100, startTime, duration);
        pattern->addNote(note);
        
        std::cout << "  Note " << (i + 1) << ": MIDI " << scaleNotes[i] 
                  << " at " << startTime << "s" << std::endl;
    }
    
    pattern->setLength(4.0);  // 4 seconds long
    pattern->setLooping(true); // Loop the pattern
    
    // Add pattern to sequencer
    midiSequencer->addClip(pattern, 0.0);
    midiSequencer->setTempo(120.0);
    
    std::cout << "\nMIDI pattern created and added to sequencer" << std::endl;
    std::cout << "Pattern length: " << pattern->getLength() << " seconds" << std::endl;
    std::cout << "Looping: " << (pattern->isLooping() ? "YES" : "NO") << std::endl;
    std::cout << "Tempo: " << midiSequencer->getTempo() << " BPM" << std::endl;
    
    // Start audio playback
    std::cout << "\n=== Starting playback ===" << std::endl;
    audioEngine->startPlayback();
    transport->play();
    
    std::cout << "Playing MIDI sequence..." << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;
    
    // Run for 16 seconds (4 loops of the 4-second pattern)
    for (int i = 0; i < 16; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        double position = transport->getPositionSeconds();
        int activeVoices = midiSynth->getActiveVoiceCount();
        
        std::cout << "Position: " << position << "s, Active voices: " << activeVoices << std::endl;
    }
    
    // Stop playback
    std::cout << "\n=== Stopping playback ===" << std::endl;
    transport->stop();
    audioEngine->stopPlayback();
    
    std::cout << "Playback stopped" << std::endl;
    
    // Test 2: Play a chord
    std::cout << "\n=== Testing chord playback ===" << std::endl;
    
    auto chordPattern = std::make_shared<MIDIPattern>();
    
    // C major chord (C, E, G)
    chordPattern->addNote(MIDINote(0, 60, 90, 0.0, 2.0));  // C
    chordPattern->addNote(MIDINote(0, 64, 90, 0.0, 2.0));  // E
    chordPattern->addNote(MIDINote(0, 67, 90, 0.0, 2.0));  // G
    
    chordPattern->setLength(2.0);
    chordPattern->setLooping(false);
    
    midiSequencer->clearClips();
    midiSequencer->addClip(chordPattern, 0.0);
    
    transport->setPosition(0.0);
    audioEngine->startPlayback();
    transport->play();
    
    std::cout << "Playing C major chord..." << std::endl;
    
    for (int i = 0; i < 3; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Active voices: " << midiSynth->getActiveVoiceCount() << std::endl;
    }
    
    transport->stop();
    audioEngine->stopPlayback();
    
    std::cout << "\n=== Test complete ===" << std::endl;
    std::cout << "Shutting down..." << std::endl;
    
    daw.shutdown();
    
    std::cout << "DAW shutdown complete" << std::endl;
    return 0;
}
