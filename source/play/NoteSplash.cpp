#include "NoteSplash.h"

#include "../data/Paths.h"
#include "../graphics/RGBPalette.h"

#include <array>
#include <cstdlib>

namespace FNF {

bool NoteSplash::s_TemplatesLoaded = false;
std::array<std::array<AnimatedSprite, 2>, 4> NoteSplash::s_Templates = {};

namespace {

const std::array<std::array<std::string, 2>, 4> kPrefixes = {{
    { "note splash purple 1", "note splash purple 2" },
    { "note splash blue 1", "note splash blue 2" },
    { "note splash green 1", "note splash green 2" },
    { "note splash red 1", "note splash red 2" }
}};

const std::array<std::array<float, 2>, 2> kOffsets = {{
    { -58.0f, -55.0f },
    { -52.0f, -48.0f }
}};

}

bool NoteSplash::EnsureTemplates(SDL_Renderer* renderer) {
    if (s_TemplatesLoaded) {
        return true;
    }

    std::string imagePath = Paths::Image("noteSplashes/noteSplashes");
    std::string xmlPath = Paths::Xml("noteSplashes/noteSplashes");
    if (imagePath.empty() || xmlPath.empty()) {
        imagePath = Paths::Image("noteSplashes");
        xmlPath = Paths::Xml("noteSplashes");
    }

    for (int lane = 0; lane < 4; ++lane) {
        for (int variant = 0; variant < 2; ++variant) {
            if (!s_Templates[lane][variant].Load(renderer, imagePath, xmlPath)) {
                return false;
            }

            s_Templates[lane][variant].AddByPrefix("splash", kPrefixes[lane][variant], 24, false);
            RGBPalette::ApplyLaneTint(s_Templates[lane][variant], lane, 1.15f + static_cast<float>(variant) * 0.05f);
            s_Templates[lane][variant].Play("splash", true);
        }
    }

    s_TemplatesLoaded = true;
    return true;
}

bool NoteSplash::Spawn(SDL_Renderer* renderer, const StrumNote& strum, int lane) {
    if (!EnsureTemplates(renderer)) {
        return false;
    }

    const int variant = std::rand() % 2;
    m_Sprite = s_Templates[lane][variant];
    m_Sprite.Play("splash", true);
    m_Sprite.x = strum.GetCenterX() - m_Sprite.GetWidth() * 0.5f + kOffsets[variant][0];
    m_Sprite.y = strum.GetCenterY() - m_Sprite.GetHeight() * 0.5f + kOffsets[variant][1];
    m_Alive = true;
    return true;
}

void NoteSplash::Update(float dt) {
    if (!m_Alive) {
        return;
    }

    m_Sprite.Update(dt);
    if (m_Sprite.IsFinished()) {
        m_Alive = false;
    }
}

void NoteSplash::Draw(SDL_Renderer* renderer) const {
    if (m_Alive) {
        m_Sprite.Draw(renderer);
    }
}

void NoteSplash::InvalidateSharedResources() {
    s_TemplatesLoaded = false;
    s_Templates = {};
}

} // namespace FNF