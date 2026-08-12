/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Conductor - BPM and Beat Tracking System
 * 
 * Mirrors Conductor.hx: tracks song position, beats, steps and BPM changes.
 * The most critical timing class in the whole engine.
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#pragma once

#include <vector>

namespace FNF {

struct BPMChangeEvent {
    int   stepTime    = 0;
    float songTime    = 0.0f;
    float bpm         = 100.0f;
    float stepCrochet = 0.0f;  // ms per step at this BPM
};

class Conductor {
public:
    static float bpm;
    static float crochet;       // ms per beat   = (60 / bpm) * 1000
    static float stepCrochet;   // ms per step   = crochet / 4
    static float songPosition;  // current ms (from MusicPlayer)
    static float offset;        // global audio offset in ms

    static std::vector<BPMChangeEvent> bpmChangeMap;

    /**
     * Change the active BPM - recalculates crochet and stepCrochet.
     */
    static void SetBPM(float newBPM);

    /**
     * Sync songPosition from MusicPlayer each frame.
     * Call this at the start of every Update().
     */
    static void Update();

    // -----------------------------------------------------------------------
    // Helpers (mirrors Conductor.hx statics)
    // -----------------------------------------------------------------------

    static float GetStep(float time);
    static float GetBeat(float time);
    static int   GetStepRounded(float time);
    static int   GetBeatRounded(float time);

    /**
     * Return the BPMChangeEvent active at a given time in ms.
     */
    static BPMChangeEvent GetBPMFromSeconds(float time);

    /**
     * Map BPM changes from a song's bpmChanges array.
     * Call once when loading a chart.
     */
    static void MapBPMChanges(const std::vector<BPMChangeEvent>& changes);
    static void ClearBPMChanges();
};

} // namespace FNF
