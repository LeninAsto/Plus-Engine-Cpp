/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Minimal song chart loader for LoadingState and PlayState.
 */

#pragma once

#include <optional>
#include <string>
#include <vector>

namespace FNF {

struct ChartNote {
    float strumTime = 0.0f;
    int lane = 0;
    float sustainLength = 0.0f;
    bool mustHit = false;
};

struct SongChartData {
    std::string songName;
    std::string player1 = "bf";
    std::string player2 = "dad";
    std::string gfVersion = "gf";
    std::string stage;
    float bpm = 100.0f;
    bool needsVoices = false;
    std::vector<ChartNote> notes;
};

class SongChart {
public:
    static std::optional<SongChartData> LoadFromFile(const std::string& chartPath);
};

} // namespace FNF