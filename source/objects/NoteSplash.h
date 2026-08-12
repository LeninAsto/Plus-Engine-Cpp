#pragma once

#include "StrumNote.h"

#include <array>

namespace FNF {

class OpenGLESBackend;

class NoteSplash {
public:
    bool Spawn(SDL_Renderer* renderer, const StrumNote& strum, int lane);
    bool SpawnGL(const StrumNote& strum, int lane);
    void Update(float dt);
    void Draw(SDL_Renderer* renderer) const;
    void DrawGL(OpenGLESBackend& backend) const;
    bool IsAlive() const { return m_Alive; }
    static void InvalidateSharedResources();

private:
    static bool EnsureTemplates(SDL_Renderer* renderer);
    static bool EnsureTemplatesGL();

    AnimatedSprite m_Sprite;
    bool m_Alive = false;

    static bool s_TemplatesLoaded;
    static bool s_TemplatesLoadedGL;
    static std::array<std::array<AnimatedSprite, 2>, 4> s_Templates;
};

} // namespace FNF
