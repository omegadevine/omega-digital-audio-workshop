# DAW Sequencer Design Research & Best Practices

## Executive Summary
Based on research of industry-leading DAWs (Ableton Live, FL Studio, Logic Pro), this document outlines best practices for sequencer timeline design and identifies improvements needed for Omega DAW.

---

## Key Design Principles from Top DAWs

### 1. Layout & Visual Design

**Ableton Live**
- Dual interface: Session View (clip launching) + Arrangement View (linear timeline)
- Minimalist, clean design - shows only essential controls
- Clear visual hierarchy between tracks, clips, and timeline
- Color-coding for quick identification

**FL Studio**
- Modular approach: Channel Rack, Piano Roll, Playlist
- Highly customizable UI with drag-and-drop workflow
- Strong visual feedback for all operations
- Pattern-based + timeline arrangement

**Logic Pro**
- Streamlined, professional workspace
- Track stacks for organization
- Integrated region/clip editing
- Smart Tempo & Flex Time for audio warping

### 2. Essential Features

#### Snap to Grid
- **Toggle on/off** with keyboard shortcut
- **Flexible grid divisions**: bars, beats, triplets, dotted notes, frames, samples
- **Smart snap**: adjust resolution based on zoom level
- **Magnetic snap strength**: adjustable intensity
- **Landmark snapping**: markers, transients, clip boundaries, zero crossings

#### Zoom Controls
- **Mouse wheel/pinch**: smooth zooming
- **Zoom to selection**: fit selected region to view
- **Zoom presets**: fit all, show project, custom views
- **Horizontal & vertical zoom**: independent control
- **Zoom follows playhead**: auto-scroll during playback

#### Clip Selection
- **Click to select**, Ctrl/Cmd for multi-select
- **Lasso/marquee selection**: drag rectangle to select multiple
- **Keyboard navigation**: arrows, Tab/Shift-Tab
- **Select all on track**: quick track-wide selection
- **Clear visual feedback**: highlight, border, color change
- **Selection grouping**: operate on multiple clips as one

#### Drag & Drop
- **Ghost preview**: show where clip will land
- **Real-time snapping feedback**: visual grid alignment
- **Cross-track dragging**: move clips between tracks
- **Modifier keys**: Copy (Ctrl), time-stretch (Alt), etc.
- **Collision detection**: prevent/warn about overlaps
- **Drop target highlighting**: show valid drop zones

### 3. Advanced Features

#### Clip Editing
- **Split/Cut**: divide clips at cursor/grid
- **Duplicate**: Ctrl+D for quick copy
- **Fade in/out**: drag clip edges for crossfades
- **Gain/Volume envelopes**: per-clip automation
- **Time-stretch/pitch**: non-destructive audio warping
- **Loop points**: set in/out for clip looping

#### Track Management
- **Track heights**: adjustable, collapsed/expanded views
- **Track colors**: user-customizable for organization
- **Track groups/folders**: organize related tracks
- **Track icons**: visual identification
- **Solo/Mute/Arm**: quick access buttons

#### Timeline Features
- **Playhead**: clear, high-contrast cursor
- **Loop region**: visual bracket with handles
- **Markers**: named positions, color-coded
- **Time signature changes**: visible in ruler
- **Tempo automation**: visual curve/ramp
- **Ruler modes**: bars/beats, time, samples, frames

---

## Current Issues in Omega DAW

### Critical Fixes Needed

1. **Clips Not Loading**
   - `Project::createDemoClips()` called before tracks exist
   - Need to create default tracks in `newProject()`
   - Tracks should be added to project, not just arrangement

2. **Save Functionality Incomplete**
   - File dialog implemented but save path not working
   - Need to wire up File → Save As to actual file picker
   - Project::save() needs valid file path

3. **No Interactive Editing**
   - Clips are displayed but can't be clicked/dragged
   - No clip selection system
   - No drag & drop implementation
   - No snap-to-grid toggle

### Medium Priority

4. **Missing Zoom Controls**
   - No zoom in/out (critical for detail work)
   - No scroll bars for timeline navigation
   - Fixed 50px/second scale (should be adjustable)

5. **Limited Track Display**
   - Only 4 tracks visible in mixer
   - No scrolling for additional tracks
   - No track height adjustment

6. **No Grid Visualization**
   - No beat markers on timeline
   - No subdivision lines (1/4, 1/8, 1/16 notes)
   - Grid should sync with project tempo

---

## Recommended Implementation Priority

### Phase 1: Core Functionality (Current Focus)
1. ✅ Basic clip data structures (AudioClip, MIDIClip)
2. ✅ Clip rendering in timeline
3. ✅ Playhead animation
4. ⚠️ **FIX: Create tracks in newProject()**
5. ⚠️ **FIX: Wire up File → Save As**

### Phase 2: Essential Interactions
6. Click to select clips
7. Drag clips horizontally (time)
8. Drag clips vertically (track change)
9. Snap to grid toggle (S key)
10. Delete selected clips (Del key)

### Phase 3: Editing Tools
11. Split clip at playhead (Cmd+E)
12. Duplicate selected clip (Cmd+D)
13. Resize clip by dragging edges
14. Clip fade in/out handles
15. Undo/Redo system

### Phase 4: Navigation & View
16. Zoom in/out (Cmd +/-, mouse wheel)
17. Horizontal scrolling (drag timeline, scroll bars)
18. Zoom to selection (F key)
19. Snap grid resolution dropdown
20. Timeline ruler with beat markers

### Phase 5: Advanced Features
21. Multi-clip selection (lasso)
22. Cut/Copy/Paste clips
23. Time stretch audio clips
24. Loop region editing
25. Automation lanes

---

## Example Code Patterns

### Snap to Grid Calculation
```cpp
double snapTimeToGrid(double time, double gridSize, bool snapEnabled) {
    if (!snapEnabled) return time;
    return std::round(time / gridSize) * gridSize;
}
```

### Clip Hit Testing
```cpp
bool isPointInClip(int mouseX, int mouseY, const Clip* clip, double pixelsPerSecond) {
    int clipX = timeToPixels(clip->getStartTime(), pixelsPerSecond);
    int clipWidth = timeToPixels(clip->getDuration(), pixelsPerSecond);
    int clipY = getTrackY(clip->getTrackIndex());
    int clipHeight = TRACK_HEIGHT;
    
    return (mouseX >= clipX && mouseX < clipX + clipWidth &&
            mouseY >= clipY && mouseY < clipY + clipHeight);
}
```

### Drag & Drop Pattern
```cpp
void handleMouseDrag(int mouseX, int mouseY) {
    if (!selectedClip || !isDragging) return;
    
    double newTime = pixelsToTime(mouseX - dragOffset, pixelsPerSecond);
    if (snapEnabled) {
        newTime = snapTimeToGrid(newTime, gridSize, true);
    }
    
    selectedClip->setStartTime(newTime);
    
    // Show ghost preview at new position
    renderClipGhost(selectedClip, newTime);
}
```

---

## References

- [Ableton Live UI Design](https://www.ableton.com/en/manual/welcome-to-live/)
- [FL Studio Workflow](https://www.image-line.com/fl-studio-learning/)
- [Logic Pro Documentation](https://support.apple.com/guide/logicpro/)
- [DAW UX Best Practices - Sound on Sound](https://www.soundonsound.com/techniques/step-sequencing)
- [Timeline Design Patterns - Universal Audio](https://help.uaudio.com/hc/en-us/articles/360041441712)

---

## Next Steps

1. **Fix critical bugs** (tracks not loading, save not working)
2. **Implement clip selection** (click detection)
3. **Add snap to grid toggle** (visual feedback)
4. **Implement basic drag & drop** (horizontal time movement)
5. **Add zoom controls** (essential for usability)

Once these are complete, Omega DAW will have a functional, usable sequencer comparable to basic modes in commercial DAWs.
