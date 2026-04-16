/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * MusicBeatState Implementation
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#include "MusicBeatState.h"
#include "../audio/Conductor.h"
#include <cmath>

namespace FNF {

void MusicBeatState::Update(float dt) {
    // Sync song position
    Conductor::Update();

    // Update decimal counters
    curDecStep = Conductor::GetStep(Conductor::songPosition);
    curDecBeat = curDecStep / 4.0f;

    // Detect whole new steps
    int newStep = static_cast<int>(std::floor(curDecStep));
    while (m_PrevStep < newStep) {
        m_PrevStep++;
        curStep = m_PrevStep;
        StepHit();

        // Every 4 steps = one beat
        int newBeat = m_PrevStep / 4;
        if (m_PrevBeat < newBeat) {
            m_PrevBeat = newBeat;
            curBeat    = m_PrevBeat;
            BeatHit();
        }
    }
}

} // namespace FNF
