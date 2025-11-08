#ifndef OMEGA_DAW_TRANSPORT_H
#define OMEGA_DAW_TRANSPORT_H

#include <cstdint>
#include <functional>

namespace OmegaDAW {

class Transport {
public:
    Transport();
    ~Transport() = default;

    void play();
    void stop();
    void pause();
    void record();
    
    bool isPlaying() const { return playing_; }
    bool isRecording() const { return recording_; }
    bool isPaused() const { return paused_; }
    
    void setTempo(double bpm);
    double getTempo() const { return tempo_; }
    
    void setTimeSignature(int numerator, int denominator);
    int getTimeSignatureNumerator() const { return timeSignatureNumerator_; }
    int getTimeSignatureDenominator() const { return timeSignatureDenominator_; }
    
    void setLooping(bool enabled);
    bool isLooping() const { return looping_; }
    
    void setLoopStart(double beats);
    void setLoopEnd(double beats);
    double getLoopStart() const { return loopStart_; }
    double getLoopEnd() const { return loopEnd_; }
    
    void setPosition(double beats);
    double getPosition() const { return positionInBeats_; }
    double getPositionSeconds() const { return positionInBeats_ / (tempo_ / 60.0); }
    
    void setSampleRate(int sampleRate);
    int getSampleRate() const { return sampleRate_; }
    
    void advance(int numSamples);
    
    using TransportCallback = std::function<void()>;
    void setPlayCallback(TransportCallback callback) { playCallback_ = callback; }
    void setStopCallback(TransportCallback callback) { stopCallback_ = callback; }
    void setPauseCallback(TransportCallback callback) { pauseCallback_ = callback; }
    void setRecordCallback(TransportCallback callback) { recordCallback_ = callback; }

private:
    bool playing_;
    bool recording_;
    bool paused_;
    bool looping_;
    
    double tempo_;
    int timeSignatureNumerator_;
    int timeSignatureDenominator_;
    
    double positionInBeats_;
    double loopStart_;
    double loopEnd_;
    
    int sampleRate_;
    int64_t positionInSamples_;
    
    TransportCallback playCallback_;
    TransportCallback stopCallback_;
    TransportCallback pauseCallback_;
    TransportCallback recordCallback_;
    
    double samplesToBeats(int64_t samples) const;
    int64_t beatsToSamples(double beats) const;
    
public:
    // Integration methods
    void initialize();
    void shutdown();
    void reset();
    void advance();
};

} // namespace OmegaDAW

#endif // OMEGA_DAW_TRANSPORT_H
