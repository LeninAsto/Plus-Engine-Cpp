/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Conductor Implementation
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#include "Conductor.h"
#include "MusicPlayer.h"
#include <cmath>

namespace FNF {

// Static initialization
float Conductor::bpm          = 100.0f;
float Conductor::crochet      = (60.0f / 100.0f) * 1000.0f;
float Conductor::stepCrochet  = crochet / 4.0f;
float Conductor::songPosition = 0.0f;
float Conductor::offset       = 0.0f;
std::vector<BPMChangeEvent> Conductor::bpmChangeMap;

void Conductor::SetBPM(float newBPM) {
    bpm         = newBPM;
    crochet     = (60.0f / bpm) * 1000.0f;
    stepCrochet = crochet / 4.0f;
}

void Conductor::Update() {
    if (MusicPlayer::IsPlaying()) {
        songPosition = static_cast<float>(MusicPlayer::GetPositionMs()) + offset;
    }
    // If not playing, songPosition stays where it was (no advancement needed)
}

BPMChangeEvent Conductor::GetBPMFromSeconds(float time) {
    BPMChangeEvent last;
    last.stepTime    = 0;
    last.songTime    = 0.0f;
    last.bpm         = bpm;
    last.stepCrochet = stepCrochet;

    for (const auto& change : bpmChangeMap) {
        if (time >= change.songTime) {
            last = change;
        }
    }
    return last;
}

float Conductor::GetStep(float time) {
    auto last = GetBPMFromSeconds(time);
    return last.stepTime + (time - last.songTime) / last.stepCrochet;
}

float Conductor::GetBeat(float time) {
    return GetStep(time) / 4.0f;
}

int Conductor::GetStepRounded(float time) {
    auto last = GetBPMFromSeconds(time);
    return last.stepTime + static_cast<int>(std::floor(time - last.songTime) / last.stepCrochet);
}

int Conductor::GetBeatRounded(float time) {
    return static_cast<int>(std::floor(GetStep(time) / 4.0f));
}

void Conductor::MapBPMChanges(const std::vector<BPMChangeEvent>& changes) {
    bpmChangeMap = changes;
}

void Conductor::ClearBPMChanges() {
    bpmChangeMap.clear();
}

} // namespace FNF
