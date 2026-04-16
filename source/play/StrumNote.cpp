#include "StrumNote.h"

#include "../core/Logger.h"
#include "../data/Paths.h"
#include "../graphics/RGBPalette.h"

#include <array>

namespace FNF {

namespace {

const std::array<std::string, 4> kStaticPrefixes = {
    "arrowLEFT",
    "arrowDOWN",
    "arrowUP",
    "arrowRIGHT"
};

const std::array<std::string, 4> kPressPrefixes = {
    "left press",
    "down press",
    "up press",
    "right press"
};

const std::array<std::string, 4> kConfirmPrefixes = {
    "left confirm",
    "down confirm",
    "up confirm",
    "right confirm"
};

}

bool StrumNote::Load(SDL_Renderer* renderer, int lane, bool player) {
    m_Lane = lane;
    m_Player = player;
    m_Pressed = false;
    m_ResetTimer = 0.0f;
    m_LogicalX = 0.0f;
    m_LogicalY = 0.0f;
    m_LogicalWidth = 0.0f;
    m_LogicalHeight = 0.0f;

    std::string imagePath = Paths::Image("noteSkins/NOTE_assets");
    std::string xmlPath = Paths::Xml("noteSkins/NOTE_assets");
    if (imagePath.empty() || xmlPath.empty()) {
        imagePath = Paths::Image("NOTE_assets");
        xmlPath = Paths::Xml("NOTE_assets");
    }

    if (!m_Sprite.Load(renderer, imagePath, xmlPath)) {
        return false;
    }

    m_BaseTexture = TextureCache::Load(renderer, imagePath);
    m_PaletteTexture = RGBPalette::LoadNoteAtlas(renderer, imagePath, lane);
    if (!m_BaseTexture || !m_PaletteTexture) {
        return false;
    }

    const RGBPalette::LanePaletteColors palette = RGBPalette::LanePalette(lane);
    Logger::Info("[StrumNote] lane=" + std::to_string(lane)
        + " static=baseAtlas active=paletteAtlas rgb=("
        + std::to_string(palette.redChannel.r) + "," + std::to_string(palette.redChannel.g) + "," + std::to_string(palette.redChannel.b) + ") glow=("
        + std::to_string(palette.greenChannel.r) + "," + std::to_string(palette.greenChannel.g) + "," + std::to_string(palette.greenChannel.b) + ") outline=("
        + std::to_string(palette.blueChannel.r) + "," + std::to_string(palette.blueChannel.g) + "," + std::to_string(palette.blueChannel.b) + ")");

    m_Sprite.scaleX = kScale;
    m_Sprite.scaleY = kScale;
    m_Sprite.AddByPrefix("static", kStaticPrefixes[lane], 24, false);
    m_Sprite.AddByPrefix("pressed", kPressPrefixes[lane], 24, false);
    m_Sprite.AddByPrefix("confirm", kConfirmPrefixes[lane], 24, false);
    m_Sprite.Play("static", true);
    m_LogicalWidth = m_Sprite.GetWidth();
    m_LogicalHeight = m_Sprite.GetHeight();
    m_Sprite.SetTexture(m_BaseTexture);
    RGBPalette::ApplyNeutralBrightness(m_Sprite, 1.0f);
    UpdateVisualPlacement();
    return true;
}

void StrumNote::SetPosition(float x, float y) {
    m_LogicalX = x;
    m_LogicalY = y;
    UpdateVisualPlacement();
}

void StrumNote::Update(float dt) {
    if (m_ResetTimer > 0.0f) {
        m_ResetTimer -= dt;
        if (m_ResetTimer <= 0.0f) {
            PlayAnim(m_Pressed ? "pressed" : "static", true);
            m_ResetTimer = 0.0f;
        }
    }

    m_Sprite.Update(dt);
    UpdateVisualPlacement();
}

void StrumNote::Draw(SDL_Renderer* renderer) const {
    m_Sprite.Draw(renderer);
}

void StrumNote::Press() {
    m_Pressed = true;
    if (m_ResetTimer <= 0.0f) {
        PlayAnim("pressed", true);
    }
}

void StrumNote::Release() {
    m_Pressed = false;
    if (m_ResetTimer <= 0.0f) {
        PlayAnim("static", true);
    }
}

void StrumNote::Confirm(float duration) {
    m_Pressed = false;
    m_ResetTimer = duration;
    PlayAnim("confirm", true);
}

SDL_Color StrumNote::GetLaneColor() const {
    return RGBPalette::LaneColor(m_Lane);
}

void StrumNote::ApplyLanePalette(const std::string& animName) {
    if (animName == "static") {
        m_Sprite.SetTexture(m_BaseTexture);
        RGBPalette::ApplyNeutralBrightness(m_Sprite, 1.0f);
        return;
    }

    m_Sprite.SetTexture(m_PaletteTexture);
    float brightness = 1.0f;
    if (animName == "confirm") {
        brightness = 1.08f;
    }
    RGBPalette::ApplyNeutralBrightness(m_Sprite, brightness);
}

void StrumNote::PlayAnim(const std::string& name, bool forceRestart) {
    m_Sprite.Play(name, forceRestart);
    ApplyLanePalette(name);
    UpdateVisualPlacement();
}

void StrumNote::UpdateVisualPlacement() {
    const float currentWidth = m_Sprite.GetWidth();
    const float currentHeight = m_Sprite.GetHeight();

    if (m_LogicalWidth <= 0.0f) {
        m_LogicalWidth = currentWidth;
    }
    if (m_LogicalHeight <= 0.0f) {
        m_LogicalHeight = currentHeight;
    }

    m_Sprite.x = m_LogicalX + (m_LogicalWidth - currentWidth) * 0.5f;
    m_Sprite.y = m_LogicalY + (m_LogicalHeight - currentHeight) * 0.5f;
}

} // namespace FNF