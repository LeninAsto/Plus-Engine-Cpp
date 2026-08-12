/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * MusicBeatState - State with BPM-synchronized Beat/Step Callbacks
 * 
 * Mirrors MusicBeatState.hx: provides curBeat, curStep, curDecBeat,
 * curDecStep. Subclasses override beatHit() and stepHit() for sync effects.
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#pragma once

#include "State.h"

namespace FNF {

class MusicBeatState : public State {
public:
    // -----------------------------------------------------------------------
    // Beat / step counters (mirrors MusicBeatState.hx)
    // -----------------------------------------------------------------------
    int   curBeat    = 0;
    int   curStep    = 0;
    float curDecBeat = 0.0f;
    float curDecStep = 0.0f;

    // -----------------------------------------------------------------------
    // State overrides
    // -----------------------------------------------------------------------
    void Update(float dt) override;

protected:
    /**
     * Called once every 4 steps (once per beat).
     * Override in subclasses for per-beat effects (e.g., logo bump).
     */
    virtual void BeatHit() {}

    /**
     * Called once every step (4 per beat).
     * Override for per-step effects (e.g., character bob).
     */
    virtual void StepHit() {}

private:
    int m_PrevBeat = -1;
    int m_PrevStep = -1;
};

} // namespace FNF
