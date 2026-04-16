/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Minimal stage scene loader.
 */

#pragma once

#include "../graphics/Sprite.h"
#include <SDL2/SDL.h>
#include <optional>
#include <string>
#include <vector>

namespace FNF {

struct StageData {
    std::string stageName = "stage";
    std::string directory = "week1";
    float defaultZoom = 0.9f;
    float boyfriendX = 770.0f;
    float boyfriendY = 100.0f;
    float opponentX = 100.0f;
    float opponentY = 100.0f;
    float girlfriendX = 400.0f;
    float girlfriendY = 130.0f;
    bool hideGirlfriend = false;
    std::vector<std::string> preloadImages;
};

class StageScene {
public:
    static std::optional<StageData> LoadData(const std::string& stageName);
    static bool Precache(SDL_Renderer* renderer, const StageData& data);

    bool Load(SDL_Renderer* renderer, const StageData& data);
    void Draw(SDL_Renderer* renderer) const;

private:
    Sprite m_Back;
    Sprite m_Front;
    Sprite m_Curtains;
};

} // namespace FNF