#ifndef UITIMELINE_H
#define UITIMELINE_H

#include "UIWindow.h"
#include "Track.h"
#include <vector>
#include <memory>

namespace OmegaDAW {

struct TimelineClip {
    std::string trackId;
    double startTime;
    double duration;
    Color color;
    std::string name;
    bool selected;
    
    TimelineClip() : startTime(0), duration(0), selected(false) {}
};

class UITimeline : public UIComponent {
private:
    double viewStartTime;
    double viewEndTime;
    double pixelsPerSecond;
    int numTracks;
    float trackHeight;
    
    std::vector<TimelineClip> clips;
    TimelineClip* selectedClip;
    TimelineClip* draggingClip;
    
    double playheadPosition;
    bool showGrid;
    double gridInterval;
    
    float dragStartX;
    double dragStartTime;
    
public:
    UITimeline(const std::string& id);
    
    void render() override;
    void handleMouseDown(float x, float y) override;
    void handleMouseUp(float x, float y) override;
    void handleMouseMove(float x, float y) override;
    void update(float deltaTime) override;
    
    void addClip(const TimelineClip& clip);
    void removeClip(const std::string& trackId, double startTime);
    void clearClips();
    
    void setViewRange(double start, double end);
    void setNumTracks(int num) { numTracks = num; }
    void setTrackHeight(float height) { trackHeight = height; }
    
    void setPlayheadPosition(double pos) { playheadPosition = pos; }
    double getPlayheadPosition() const { return playheadPosition; }
    
    void setShowGrid(bool show) { showGrid = show; }
    void setGridInterval(double interval) { gridInterval = interval; }
    
    void zoomIn();
    void zoomOut();
    void scrollHorizontal(float delta);
    
private:
    double screenXToTime(float x) const;
    float timeToScreenX(double time) const;
    int screenYToTrack(float y) const;
    float trackToScreenY(int track) const;
};

}

#endif
