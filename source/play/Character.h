/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Minimal animated character wrapper.
 */

#pragma once

#include "../graphics/AnimatedSprite.h"
#include <SDL2/SDL.h>
#include <string>
#include <vector>

namespace FNF {

class Character {
public:
    bool Load(SDL_Renderer* renderer, const std::string& characterId);
    bool Precache(SDL_Renderer* renderer, const std::string& characterId);

    void Update(float dt);
    void Draw(SDL_Renderer* renderer) const;
    void Dance();
    void Sing(int lane);
    void SetPosition(float px, float py);

    float x = 0.0f;
    float y = 0.0f;

private:
    struct AnimationDef {
        std::string anim;
        std::string name;
        std::vector<int> indices;
        int fps = 24;
        bool loop = false;
        float offX = 0.0f;
        float offY = 0.0f;
    };

    AnimatedSprite m_Sprite;
    std::vector<AnimationDef> m_Animations;
    float m_PosOffsetX = 0.0f;
    float m_PosOffsetY = 0.0f;
    float m_CurrentAnimOffsetX = 0.0f;
    float m_CurrentAnimOffsetY = 0.0f;
    float m_Scale = 1.0f;
    bool m_DanceToggle = false;

    bool LoadInternal(SDL_Renderer* renderer, const std::string& characterId, bool keepConfiguredSprite);
    bool HasAnimation(const std::string& animName) const;
    void PlayAnimation(const std::string& animName, bool forceRestart);
};

} // namespace FNF