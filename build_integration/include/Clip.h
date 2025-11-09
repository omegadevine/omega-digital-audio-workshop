#pragma once

#include <vector>
#include <string>
#include <memory>
#include "AudioBuffer.h"
#include "MIDIMessage.h"

namespace OmegaDAW {

enum class ClipType {
    Audio,
    MIDI,
    Automation
};

class Clip {
public:
    Clip(ClipType type, double startTime, double duration);
    virtual ~Clip() = default;

    ClipType getType() const { return m_type; }
    double getStartTime() const { return m_startTime; }
    double getDuration() const { return m_duration; }
    double getEndTime() const { return m_startTime + m_duration; }
    
    void setStartTime(double time) { m_startTime = time; }
    void setDuration(double duration) { m_duration = duration; }
    void setOffset(double offset) { m_offset = offset; }
    void setLoop(bool loop) { m_loop = loop; }
    void setFadeIn(double duration) { m_fadeInDuration = duration; }
    void setFadeOut(double duration) { m_fadeOutDuration = duration; }
    void setGain(float gain) { m_gain = gain; }
    
    double getOffset() const { return m_offset; }
    bool isLooping() const { return m_loop; }
    float getGain() const { return m_gain; }
    
    void setName(const std::string& name) { m_name = name; }
    std::string getName() const { return m_name; }
    
    void setColor(unsigned int color) { m_color = color; }
    unsigned int getColor() const { return m_color; }
    
    bool isInRange(double time) const;
    float getEnvelopeAtTime(double time) const;

protected:
    ClipType m_type;
    double m_startTime;
    double m_duration;
    double m_offset;
    bool m_loop;
    float m_gain;
    double m_fadeInDuration;
    double m_fadeOutDuration;
    std::string m_name;
    unsigned int m_color;
};

class AudioClip : public Clip {
public:
    AudioClip(double startTime, double duration);
    
    void setAudioData(std::shared_ptr<AudioBuffer> buffer);
    std::shared_ptr<AudioBuffer> getAudioData() const { return m_audioData; }
    
    void setSourceFile(const std::string& filepath) { m_sourceFile = filepath; }
    std::string getSourceFile() const { return m_sourceFile; }
    
    void setPitch(float semitones) { m_pitchShift = semitones; }
    float getPitch() const { return m_pitchShift; }
    
    void setReverse(bool reverse) { m_reverse = reverse; }
    bool isReversed() const { return m_reverse; }

private:
    std::shared_ptr<AudioBuffer> m_audioData;
    std::string m_sourceFile;
    float m_pitchShift;
    bool m_reverse;
};

class MIDIClip : public Clip {
public:
    MIDIClip(double startTime, double duration);
    
    void addNote(const MIDIMessage& note);
    void removeNote(size_t index);
    void clearNotes();
    
    const std::vector<MIDIMessage>& getNotes() const { return m_notes; }
    std::vector<MIDIMessage> getNotesInRange(double startTime, double endTime) const;
    
    void quantize(double gridSize);
    void transpose(int semitones);
    void setVelocity(uint8_t velocity);

private:
    std::vector<MIDIMessage> m_notes;
};

class AutomationClip : public Clip {
public:
    struct AutomationPoint {
        double time;
        float value;
        
        AutomationPoint(double t, float v) : time(t), value(v) {}
    };
    
    AutomationClip(double startTime, double duration);
    
    void addPoint(double time, float value);
    void removePoint(size_t index);
    void clearPoints();
    
    float getValueAtTime(double time) const;
    const std::vector<AutomationPoint>& getPoints() const { return m_points; }
    
    void setTargetParameter(const std::string& target) { m_targetParameter = target; }
    std::string getTargetParameter() const { return m_targetParameter; }

private:
    std::vector<AutomationPoint> m_points;
    std::string m_targetParameter;
};

} // namespace OmegaDAW
