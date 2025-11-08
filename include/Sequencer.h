#pragma once

#include <vector>
#include <memory>
#include <functional>
#include "Arrangement.h"
#include "Transport.h"
#include "AudioEngine.h"
#include "MIDISequencer.h"

namespace OmegaDAW {

class Sequencer {
public:
    Sequencer(AudioEngine& audioEngine);
    ~Sequencer() = default;

    void setArrangement(std::shared_ptr<Arrangement> arrangement);
    std::shared_ptr<Arrangement> getArrangement() const { return m_arrangement; }
    
    void setTransport(std::shared_ptr<Transport> transport);
    std::shared_ptr<Transport> getTransport() const { return m_transport; }
    
    void process(double deltaTime);
    
    void scheduleClipsForPlayback();
    void stopAllClips();
    
    void setQuantization(double quantize) { m_quantization = quantize; }
    double getQuantization() const { return m_quantization; }
    
    void setRecording(bool recording) { m_recording = recording; }
    bool isRecording() const { return m_recording; }
    
    void setRecordTrack(size_t trackIndex) { m_recordTrackIndex = trackIndex; }
    size_t getRecordTrack() const { return m_recordTrackIndex; }
    
    void recordAudio(const AudioBuffer& buffer);
    void recordMIDI(const MIDIMessage& message);
    
    void setPunch(bool enabled, double punchIn, double punchOut);
    bool isPunchEnabled() const { return m_punchEnabled; }
    
    void setMetronome(bool enabled) { m_metronomeEnabled = enabled; }
    bool isMetronomeEnabled() const { return m_metronomeEnabled; }
    
    void setCountIn(bool enabled, int bars) { m_countInEnabled = enabled; m_countInBars = bars; }
    bool isCountInEnabled() const { return m_countInEnabled; }
    
    using PlayheadCallback = std::function<void(double)>;
    void setPlayheadCallback(PlayheadCallback callback) { m_playheadCallback = callback; }

private:
    void processAudioClips(double currentTime, double deltaTime);
    void processMIDIClips(double currentTime, double deltaTime);
    void processAutomation(double currentTime);
    void processMetronome(double currentTime);
    void handleLooping(double& currentTime);
    
    AudioEngine& m_audioEngine;
    std::shared_ptr<Arrangement> m_arrangement;
    std::shared_ptr<Transport> m_transport;
    
    double m_quantization;
    bool m_recording;
    size_t m_recordTrackIndex;
    
    bool m_punchEnabled;
    double m_punchIn;
    double m_punchOut;
    
    bool m_metronomeEnabled;
    int m_currentBeat;
    double m_lastBeatTime;
    
    bool m_countInEnabled;
    int m_countInBars;
    int m_countInBeatsRemaining;
    
    std::shared_ptr<AudioClip> m_recordingClip;
    std::shared_ptr<MIDIClip> m_recordingMIDIClip;
    
    PlayheadCallback m_playheadCallback;
};

} // namespace OmegaDAW
