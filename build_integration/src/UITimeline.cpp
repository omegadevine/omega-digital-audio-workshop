#include "UITimeline.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace OmegaDAW {

UITimeline::UITimeline(const std::string& id)
    : UIComponent(id), viewStartTime(0.0), viewEndTime(60.0),
      pixelsPerSecond(50.0), numTracks(8), trackHeight(60.0f),
      selectedClip(nullptr), draggingClip(nullptr),
      playheadPosition(0.0), showGrid(true), gridInterval(1.0),
      dragStartX(0), dragStartTime(0) {
    backgroundColor = Color(0.18f, 0.18f, 0.18f, 1.0f);
    foregroundColor = Color(0.8f, 0.8f, 0.8f, 1.0f);
}

void UITimeline::render() {
    // Render timeline background
    // Render grid if enabled
    if (showGrid) {
        double gridTime = std::ceil(viewStartTime / gridInterval) * gridInterval;
        while (gridTime <= viewEndTime) {
            float x = timeToScreenX(gridTime);
            // Draw vertical grid line at x
            gridTime += gridInterval;
        }
    }
    
    // Render track separators
    for (int i = 0; i <= numTracks; i++) {
        float y = trackToScreenY(i);
        // Draw horizontal line at y
    }
    
    // Render clips
    for (const auto& clip : clips) {
        float x = timeToScreenX(clip.startTime);
        float width = static_cast<float>((clip.duration * pixelsPerSecond));
        int trackIndex = 0; // Would map from clip.trackId
        float y = trackToScreenY(trackIndex);
        
        // Draw clip rectangle with clip.color
        // Draw clip name
        // Draw selection highlight if clip.selected
    }
    
    // Render playhead
    float playheadX = timeToScreenX(playheadPosition);
    // Draw vertical line for playhead
}

void UITimeline::update(float deltaTime) {
    // Update animations or playhead if playing
}

void UITimeline::handleMouseDown(float x, float y) {
    if (!bounds.contains(x, y) || !enabled) return;
    
    double clickTime = screenXToTime(x);
    int clickTrack = screenYToTrack(y);
    
    // Check if clicked on a clip
    selectedClip = nullptr;
    for (auto& clip : clips) {
        if (clickTime >= clip.startTime && 
            clickTime <= clip.startTime + clip.duration) {
            selectedClip = &clip;
            draggingClip = &clip;
            clip.selected = true;
            dragStartX = x;
            dragStartTime = clip.startTime;
            break;
        } else {
            clip.selected = false;
        }
    }
    
    // If no clip clicked, position playhead
    if (!selectedClip) {
        playheadPosition = clickTime;
    }
}

void UITimeline::handleMouseUp(float x, float y) {
    draggingClip = nullptr;
}

void UITimeline::handleMouseMove(float x, float y) {
    if (draggingClip) {
        double deltaTime = screenXToTime(x) - screenXToTime(dragStartX);
        draggingClip->startTime = dragStartTime + deltaTime;
        
        // Snap to grid if enabled
        if (showGrid) {
            draggingClip->startTime = std::round(draggingClip->startTime / gridInterval) * gridInterval;
        }
    }
}

void UITimeline::addClip(const TimelineClip& clip) {
    clips.push_back(clip);
}

void UITimeline::removeClip(const std::string& trackId, double startTime) {
    clips.erase(
        std::remove_if(clips.begin(), clips.end(),
            [&](const TimelineClip& clip) {
                return clip.trackId == trackId && 
                       std::abs(clip.startTime - startTime) < 0.001;
            }),
        clips.end()
    );
}

void UITimeline::clearClips() {
    clips.clear();
    selectedClip = nullptr;
    draggingClip = nullptr;
}

void UITimeline::setViewRange(double start, double end) {
    viewStartTime = start;
    viewEndTime = end;
    pixelsPerSecond = bounds.width / (end - start);
}

void UITimeline::zoomIn() {
    double center = (viewStartTime + viewEndTime) / 2.0;
    double range = (viewEndTime - viewStartTime) * 0.75;
    setViewRange(center - range / 2.0, center + range / 2.0);
}

void UITimeline::zoomOut() {
    double center = (viewStartTime + viewEndTime) / 2.0;
    double range = (viewEndTime - viewStartTime) * 1.33;
    setViewRange(center - range / 2.0, center + range / 2.0);
}

void UITimeline::scrollHorizontal(float delta) {
    double timeShift = delta / pixelsPerSecond;
    setViewRange(viewStartTime + timeShift, viewEndTime + timeShift);
}

double UITimeline::screenXToTime(float x) const {
    float relativeX = x - bounds.x;
    return viewStartTime + (relativeX / pixelsPerSecond);
}

float UITimeline::timeToScreenX(double time) const {
    return bounds.x + static_cast<float>((time - viewStartTime) * pixelsPerSecond);
}

int UITimeline::screenYToTrack(float y) const {
    float relativeY = y - bounds.y;
    return static_cast<int>(relativeY / trackHeight);
}

float UITimeline::trackToScreenY(int track) const {
    return bounds.y + track * trackHeight;
}

}
