#pragma once

#include "StrumNote.h"

#include <array>

namespace FNF {

class HoldSplash {
public:
    bool Spawn(SDL_Renderer* renderer, const StrumNote& strum, int lane, float remainingDurationMs);
    void Update(float dt);
    void Draw(SDL_Renderer* renderer) const;
    bool IsAlive() const { return m_Alive; }
    static void InvalidateSharedResources();

private:
    static bool EnsureTemplates(SDL_Renderer* renderer);

    AnimatedSprite m_Sprite;
    const StrumNote* m_Strum = nullptr;
    bool m_Alive = false;
    bool m_Ending = false;
    float m_RemainingMs = 0.0f;

    static bool s_TemplatesLoaded;
    static std::array<AnimatedSprite, 4> s_HoldTemplates;
};

} // namespace FNF