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
    struct ObjectData {
        std::string image;
        float x = 0.0f;
        float y = 0.0f;
        float scaleX = 1.0f;
        float scaleY = 1.0f;
        float alpha = 1.0f;
        float angle = 0.0f;
        bool flipX = false;
        bool flipY = false;
        bool foreground = false;
        bool antialiasing = true;
    };

    std::string stageName = "stage";
    std::string directory = "week1";
    std::string stageUI;
    float defaultZoom = 0.9f;
    float boyfriendX = 770.0f;
    float boyfriendY = 100.0f;
    float opponentX = 100.0f;
    float opponentY = 100.0f;
    float girlfriendX = 400.0f;
    float girlfriendY = 130.0f;
    float cameraBoyfriendX = 0.0f;
    float cameraBoyfriendY = 0.0f;
    float cameraOpponentX = 0.0f;
    float cameraOpponentY = 0.0f;
    float cameraGirlfriendX = 0.0f;
    float cameraGirlfriendY = 0.0f;
    float cameraSpeed = 1.0f;
    bool hideGirlfriend = false;
    std::vector<std::string> preloadImages;
    std::vector<ObjectData> objects;
};

class StageScene {
public:
    static std::optional<StageData> LoadData(const std::string& stageName);
    static bool Precache(SDL_Renderer* renderer, const StageData& data);
    static std::string VanillaSongStage(const std::string& songName);

    bool Load(SDL_Renderer* renderer, const StageData& data);
    void Draw(SDL_Renderer* renderer) const;
    void Draw(SDL_Renderer* renderer, float cameraX, float cameraY, float zoom) const;

private:
    struct StageSprite {
        Sprite sprite;
        bool foreground = false;
    };

    std::vector<StageSprite> m_Sprites;
};

} // namespace FNF