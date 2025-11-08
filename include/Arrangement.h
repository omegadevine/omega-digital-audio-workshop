#pragma once

#include <vector>
#include <memory>
#include <string>
#include "Clip.h"
#include "Track.h"
#include "Transport.h"

namespace OmegaDAW {

class Arrangement {
public:
    Arrangement();
    ~Arrangement() = default;

    void addClip(size_t trackIndex, std::shared_ptr<Clip> clip);
    void removeClip(size_t trackIndex, size_t clipIndex);
    void moveClip(size_t trackIndex, size_t clipIndex, double newStartTime);
    void resizeClip(size_t trackIndex, size_t clipIndex, double newDuration);
    void splitClip(size_t trackIndex, size_t clipIndex, double splitTime);
    std::shared_ptr<Clip> duplicateClip(size_t trackIndex, size_t clipIndex);
    
    std::vector<std::shared_ptr<Clip>> getClipsOnTrack(size_t trackIndex) const;
    std::vector<std::shared_ptr<Clip>> getClipsInTimeRange(size_t trackIndex, double startTime, double endTime) const;
    std::shared_ptr<Clip> getClipAt(size_t trackIndex, double time) const;
    
    void setLoop(bool enabled, double loopStart, double loopEnd);
    bool isLoopEnabled() const { return m_loopEnabled; }
    double getLoopStart() const { return m_loopStart; }
    double getLoopEnd() const { return m_loopEnd; }
    
    void setGridSize(double size) { m_gridSize = size; }
    double getGridSize() const { return m_gridSize; }
    void setSnapToGrid(bool snap) { m_snapToGrid = snap; }
    bool getSnapToGrid() const { return m_snapToGrid; }
    
    double snapTimeToGrid(double time) const;
    
    void setTotalDuration(double duration) { m_totalDuration = duration; }
    double getTotalDuration() const { return m_totalDuration; }
    
    void clear();
    
    // Integration methods
    void initialize();
    void start();
    void stop();
    void shutdown();
    AudioBuffer renderAtPosition(double position);
    void loadFromProject(class Project* project);
    std::string serialize() const;
    
    struct Marker {
        double time;
        std::string name;
        unsigned int color;
        
        Marker(double t, const std::string& n, unsigned int c = 0xFFFFFFFF)
            : time(t), name(n), color(c) {}
    };
    
    void addMarker(const Marker& marker);
    void removeMarker(size_t index);
    const std::vector<Marker>& getMarkers() const { return m_markers; }
    
    struct TimeSignatureChange {
        double time;
        int numerator;
        int denominator;
        
        TimeSignatureChange(double t, int num, int denom)
            : time(t), numerator(num), denominator(denom) {}
    };
    
    void addTimeSignatureChange(const TimeSignatureChange& change);
    void removeTimeSignatureChange(size_t index);
    const std::vector<TimeSignatureChange>& getTimeSignatureChanges() const { return m_timeSignatureChanges; }
    TimeSignatureChange getTimeSignatureAt(double time) const;

private:
    struct ClipEntry {
        size_t trackIndex;
        std::shared_ptr<Clip> clip;
    };
    
    std::vector<ClipEntry> m_clips;
    std::vector<Marker> m_markers;
    std::vector<TimeSignatureChange> m_timeSignatureChanges;
    
    bool m_loopEnabled;
    double m_loopStart;
    double m_loopEnd;
    double m_gridSize;
    bool m_snapToGrid;
    double m_totalDuration;
};

} // namespace OmegaDAW
