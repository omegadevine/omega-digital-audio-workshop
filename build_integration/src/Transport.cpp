#include "Transport.h"
#include <algorithm>

namespace OmegaDAW {

Transport::Transport()
    : playing_(false)
    , recording_(false)
    , paused_(false)
    , looping_(false)
    , tempo_(120.0)
    , timeSignatureNumerator_(4)
    , timeSignatureDenominator_(4)
    , positionInBeats_(0.0)
    , loopStart_(0.0)
    , loopEnd_(4.0)
    , sampleRate_(44100)
    , positionInSamples_(0) {
}

void Transport::play() {
    if (!playing_) {
        playing_ = true;
        paused_ = false;
        if (playCallback_) {
            playCallback_();
        }
    }
}

void Transport::stop() {
    if (playing_ || paused_) {
        playing_ = false;
        recording_ = false;
        paused_ = false;
        positionInBeats_ = 0.0;
        positionInSamples_ = 0;
        if (stopCallback_) {
            stopCallback_();
        }
    }
}

void Transport::pause() {
    if (playing_) {
        playing_ = false;
        paused_ = true;
        if (pauseCallback_) {
            pauseCallback_();
        }
    }
}

void Transport::record() {
    recording_ = !recording_;
    if (recording_) {
        if (!playing_) {
            play();
        }
        if (recordCallback_) {
            recordCallback_();
        }
    }
}

void Transport::setTempo(double bpm) {
    tempo_ = std::clamp(bpm, 20.0, 999.0);
}

void Transport::setTimeSignature(int numerator, int denominator) {
    timeSignatureNumerator_ = std::clamp(numerator, 1, 32);
    timeSignatureDenominator_ = std::clamp(denominator, 1, 32);
}

void Transport::setLooping(bool enabled) {
    looping_ = enabled;
}

void Transport::setLoopStart(double beats) {
    loopStart_ = std::max(0.0, beats);
}

void Transport::setLoopEnd(double beats) {
    loopEnd_ = std::max(loopStart_ + 1.0, beats);
}

void Transport::setPosition(double beats) {
    positionInBeats_ = std::max(0.0, beats);
    positionInSamples_ = beatsToSamples(positionInBeats_);
}

void Transport::setSampleRate(int sampleRate) {
    sampleRate_ = sampleRate;
}

void Transport::advance(int numSamples) {
    if (!playing_) {
        return;
    }
    
    positionInSamples_ += numSamples;
    positionInBeats_ = samplesToBeats(positionInSamples_);
    
    if (looping_ && positionInBeats_ >= loopEnd_) {
        positionInBeats_ = loopStart_;
        positionInSamples_ = beatsToSamples(positionInBeats_);
    }
}

double Transport::samplesToBeats(int64_t samples) const {
    double samplesPerBeat = (60.0 / tempo_) * sampleRate_;
    return samples / samplesPerBeat;
}

int64_t Transport::beatsToSamples(double beats) const {
    double samplesPerBeat = (60.0 / tempo_) * sampleRate_;
    return static_cast<int64_t>(beats * samplesPerBeat);
}

void Transport::initialize() {
    // Initialize transport
}

void Transport::shutdown() {
    // Shutdown transport
    stop();
}

void Transport::reset() {
    positionInBeats_ = 0.0;
    positionInSamples_ = 0;
    playing_ = false;
    recording_ = false;
    paused_ = false;
}

void Transport::advance() {
    advance(512); // Default advance by 512 samples
}

} // namespace OmegaDAW
