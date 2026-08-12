/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * SongChart implementation.
 */

#include "SongChart.h"
#include "../backend/Logger.h"
#include "../backend/JsonLoader.h"

#include <algorithm>

namespace FNF {

std::optional<SongChartData> SongChart::LoadFromFile(const std::string& chartPath) {
    auto json = JsonLoader::LoadFile(chartPath);
    if (!json.has_value()) {
        return std::nullopt;
    }

    const Json* songObj = &(*json);
    if (json->contains("song") && (*json)["song"].is_object()) {
        songObj = &(*json)["song"];
    }

    SongChartData out;
    out.songName = JsonLoader::Get(*songObj, "song", std::string("Unknown Song"));
    out.player1 = JsonLoader::Get(*songObj, "player1", std::string("bf"));
    out.player2 = JsonLoader::Get(*songObj, "player2", std::string("dad"));
    out.gfVersion = JsonLoader::Get(*songObj, "gfVersion", std::string("gf"));
    out.stage = JsonLoader::Get(*songObj, "stage", std::string());
    out.bpm = JsonLoader::Get(*songObj, "bpm", 100.0f);
    out.needsVoices = JsonLoader::Get(*songObj, "needsVoices", false);

    if (songObj->contains("notes") && (*songObj)["notes"].is_array()) {
        float sectionStartMs = 0.0f;
        float sectionBpm = std::max(1.0f, out.bpm);
        for (const auto& section : (*songObj)["notes"]) {
            if (!section.is_object()) {
                continue;
            }

            const bool mustHitSection = JsonLoader::Get(section, "mustHitSection", false);
            const int lengthInSteps = std::max(1, JsonLoader::Get(section, "lengthInSteps", 16));
            const bool changeBpm = JsonLoader::Get(section, "changeBPM", false);
            if (changeBpm) {
                sectionBpm = std::max(1.0f, JsonLoader::Get(section, "bpm", sectionBpm));
            }

            ChartSection parsedSection;
            parsedSection.startTime = sectionStartMs;
            parsedSection.lengthInSteps = lengthInSteps;
            parsedSection.mustHitSection = mustHitSection;
            parsedSection.gfSection = JsonLoader::Get(section, "gfSection", false);
            out.sections.push_back(parsedSection);

            if (!section.contains("sectionNotes") || !section["sectionNotes"].is_array()) {
                sectionStartMs += (60000.0f / sectionBpm / 4.0f) * static_cast<float>(lengthInSteps);
                continue;
            }

            for (const auto& note : section["sectionNotes"]) {
                if (!note.is_array() || note.size() < 2) {
                    continue;
                }

                ChartNote parsed;
                parsed.strumTime = note[0].get<float>();
                const int rawLane = note[1].get<int>();
                parsed.lane = ((rawLane % 4) + 4) % 4;
                parsed.sustainLength = note.size() > 2 ? note[2].get<float>() : 0.0f;
                parsed.mustHit = mustHitSection;
                if (rawLane >= 4) {
                    parsed.mustHit = !parsed.mustHit;
                }
                out.notes.push_back(parsed);
            }

            sectionStartMs += (60000.0f / sectionBpm / 4.0f) * static_cast<float>(lengthInSteps);
        }
    }

    std::stable_sort(out.notes.begin(), out.notes.end(), [](const ChartNote& a, const ChartNote& b) {
        return a.strumTime < b.strumTime;
    });

    Logger::Info("[SongChart] Loaded '" + out.songName + "' with " + std::to_string(out.notes.size()) + " notes and " + std::to_string(out.sections.size()) + " sections");
    return out;
}

} // namespace FNF
