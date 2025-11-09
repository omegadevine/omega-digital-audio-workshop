#ifndef OMEGA_DAW_MIDI_SYNTHESIZER_H
#define OMEGA_DAW_MIDI_SYNTHESIZER_H

#include "AudioEngine.h"
#include "MIDIMessage.h"
#include "Oscillator.h"
#include <vector>
#include <memory>
#include <map>

namespace OmegaDAW {

struct Voice {
    int noteNumber;
    uint8_t velocity;
    float frequency;
    float amplitude;
    float phase;
    bool active;
    double startTime;
    float envelope;
    
    Voice() : noteNumber(-1), velocity(0), frequency(0), amplitude(0), 
              phase(0), active(false), startTime(0), envelope(0) {}
};

class MIDISynthesizer : public IAudioProcessor {
public:
    MIDISynthesizer(int maxPolyphony = 16);
    ~MIDISynthesizer() override = default;
    
    // IAudioProcessor interface
    void prepare(int sampleRate, int maxBufferSize) override;
    void process(float** inputs, float** outputs, int numChannels, int numFrames) override;
    std::string getName() const override { return "MIDI Synthesizer"; }
    
    // MIDI input
    void processMIDIMessage(const MIDIMessage& message);
    void processMIDIBuffer(const MIDIBuffer& buffer);
    
    // Voice management
    void noteOn(int noteNumber, uint8_t velocity);
    void noteOff(int noteNumber);
    void allNotesOff();
    
    // Parameters
    void setWaveform(WaveformType waveform) { waveform_ = waveform; }
    WaveformType getWaveform() const { return waveform_; }
    
    void setAttack(float attack) { attack_ = attack; }
    void setDecay(float decay) { decay_ = decay; }
    void setSustain(float sustain) { sustain_ = sustain; }
    void setRelease(float release) { release_ = release; }
    
    float getAttack() const { return attack_; }
    float getDecay() const { return decay_; }
    float getSustain() const { return sustain_; }
    float getRelease() const { return release_; }
    
    void setMasterVolume(float volume) { masterVolume_ = volume; }
    float getMasterVolume() const { return masterVolume_; }
    
    // Utility
    int getActiveVoiceCount() const;
    
private:
    Voice* findFreeVoice();
    Voice* findVoice(int noteNumber);
    float generateSample(Voice& voice);
    float calculateEnvelope(Voice& voice, double currentTime);
    float noteToFrequency(int noteNumber) const;
    
    std::vector<Voice> voices_;
    int maxPolyphony_;
    int sampleRate_;
    WaveformType waveform_;
    
    // ADSR envelope parameters (in seconds)
    float attack_;
    float decay_;
    float sustain_;  // 0.0 to 1.0
    float release_;
    
    float masterVolume_;
    double currentTime_;
    double timeIncrement_;
};

} // namespace OmegaDAW

#endif // OMEGA_DAW_MIDI_SYNTHESIZER_H
