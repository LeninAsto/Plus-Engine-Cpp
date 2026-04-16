#include "HoldSplash.h"

#include "../data/Paths.h"
#include "../graphics/RGBPalette.h"

#include <array>
#include <algorithm>

namespace FNF {

bool HoldSplash::s_TemplatesLoaded = false;
std::array<AnimatedSprite, 4> HoldSplash::s_HoldTemplates = {};

namespace {

const std::array<std::string, 4> kSkinKeys = {
    "holdCovers/holdCover-Purple",
    "holdCovers/holdCover-Blue",
    "holdCovers/holdCover-Green",
    "holdCovers/holdCover-Red"
};

const std::array<std::string, 4> kHoldPrefixes = {
    "holdCoverPurple",
    "holdCoverBlue",
    "holdCoverGreen",
    "holdCoverRed"
};

const std::array<std::string, 4> kEndPrefixes = {
    "holdCoverEndPurple",
    "holdCoverEndBlue",
    "holdCoverEndGreen",
    "holdCoverEndRed"
};

constexpr float kBaseOffsetX = 106.25f;
constexpr float kBaseOffsetY = 100.0f;

}

bool HoldSplash::EnsureTemplates(SDL_Renderer* renderer) {
    if (s_TemplatesLoaded) {
        return true;
    }

    for (int lane = 0; lane < 4; ++lane) {
        const std::string imagePath = Paths::Image(kSkinKeys[lane]);
        const std::string xmlPath = Paths::Xml(kSkinKeys[lane]);
        if (!s_HoldTemplates[lane].Load(renderer, imagePath, xmlPath)) {
            return false;
        }

        s_HoldTemplates[lane].AddByPrefix("hold", kHoldPrefixes[lane], 24, true);
        s_HoldTemplates[lane].AddByPrefix("end", kEndPrefixes[lane], 24, false);
        RGBPalette::ApplyLaneTint(s_HoldTemplates[lane], lane, 1.08f);
        s_HoldTemplates[lane].Play("hold", true);
    }

    s_TemplatesLoaded = true;
    return true;
}

bool HoldSplash::Spawn(SDL_Renderer* renderer, const StrumNote& strum, int lane, float remainingDurationMs) {
    if (!EnsureTemplates(renderer)) {
        return false;
    }

    m_Sprite = s_HoldTemplates[lane];
    m_Sprite.Play("hold", true);
    m_Strum = &strum;
    m_Alive = true;
    m_Ending = false;
    m_RemainingMs = std::max(0.0f, remainingDurationMs);
    return true;
}

void HoldSplash::Update(float dt) {
    if (!m_Alive || !m_Strum) {
        return;
    }

    m_Sprite.x = m_Strum->GetX() - kBaseOffsetX;
    m_Sprite.y = m_Strum->GetY() - kBaseOffsetY;
    m_Sprite.Update(dt);

    if (!m_Ending) {
        m_RemainingMs -= dt * 1000.0f;
        if (m_RemainingMs <= 0.0f) {
            m_Ending = true;
            m_Sprite.Play("end", true);
        }
    } else if (m_Sprite.IsFinished()) {
        m_Alive = false;
    }
}

void HoldSplash::Draw(SDL_Renderer* renderer) const {
    if (m_Alive) {
        m_Sprite.Draw(renderer);
    }
}

void HoldSplash::InvalidateSharedResources() {
    s_TemplatesLoaded = false;
    s_HoldTemplates = {};
}

} // namespace FNF