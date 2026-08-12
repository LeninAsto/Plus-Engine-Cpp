/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Character implementation.
 */

#include "Character.h"
#include "../backend/Logger.h"
#include "../backend/JsonLoader.h"
#include "../backend/Paths.h"
#include "../backend/OpenGLESBackend.h"

namespace FNF {

bool Character::Load(SDL_Renderer* renderer, const std::string& characterId) {
    return LoadInternal(renderer, characterId, true);
}

bool Character::Precache(SDL_Renderer* renderer, const std::string& characterId) {
    return LoadInternal(renderer, characterId, false);
}

bool Character::LoadGL(const std::string& characterId) {
    return LoadInternalGL(characterId, true);
}

bool Character::PrecacheGL(const std::string& characterId) {
    return LoadInternalGL(characterId, false);
}

bool Character::LoadInternal(SDL_Renderer* renderer, const std::string& characterId, bool keepConfiguredSprite) {
    const std::string charPath = Paths::CharacterData(characterId);
    if (charPath.empty()) {
        Logger::Warn("[Character] Character JSON not found: " + characterId);
        return false;
    }

    auto json = JsonLoader::LoadFile(charPath);
    if (!json.has_value()) {
        return false;
    }

    const std::string imageKey = JsonLoader::Get(*json, "image", std::string());
    if (imageKey.empty()) {
        Logger::Warn("[Character] Missing image key for: " + characterId);
        return false;
    }

    std::string imagePath = Paths::Image(imageKey);
    std::string xmlPath = Paths::Xml(imageKey);
    if (imagePath.empty()) {
        imagePath = Paths::Image(imageKey, "base_game");
        xmlPath = Paths::Xml(imageKey, "base_game");
    }
    if (imagePath.empty()) {
        Logger::Warn("[Character] Character atlas not found: " + imageKey);
        return false;
    }

    AnimatedSprite sprite;
    if (!sprite.Load(renderer, imagePath, xmlPath)) {
        return false;
    }

    std::vector<AnimationDef> animations;
    if (json->contains("animations") && (*json)["animations"].is_array()) {
        for (const auto& anim : (*json)["animations"]) {
            if (!anim.is_object()) {
                continue;
            }

            AnimationDef def;
            def.anim = JsonLoader::Get(anim, "anim", std::string());
            def.name = JsonLoader::Get(anim, "name", std::string());
            def.fps = JsonLoader::Get(anim, "fps", 24);
            def.loop = JsonLoader::Get(anim, "loop", false);

            if (anim.contains("indices") && anim["indices"].is_array()) {
                for (const auto& index : anim["indices"]) {
                    def.indices.push_back(index.get<int>());
                }
            }

            if (anim.contains("offsets") && anim["offsets"].is_array() && anim["offsets"].size() >= 2) {
                def.offX = anim["offsets"][0].get<float>();
                def.offY = anim["offsets"][1].get<float>();
            }

            if (def.anim.empty() || def.name.empty()) {
                continue;
            }

            if (!def.indices.empty()) {
                sprite.AddByIndices(def.anim, def.name, def.indices, def.fps, def.loop);
            } else {
                sprite.AddByPrefix(def.anim, def.name, def.fps, def.loop);
            }

            animations.push_back(def);
        }
    }

    if (!keepConfiguredSprite) {
        return true;
    }

    m_Sprite = std::move(sprite);
    m_Animations = std::move(animations);
    m_Scale = JsonLoader::Get(*json, "scale", 1.0f);
    m_Sprite.SetScale(m_Scale);
    m_Sprite.flipX = JsonLoader::Get(*json, "flip_x", false);
    m_VocalsFile = JsonLoader::Get(*json, "vocalsFile", JsonLoader::Get(*json, "vocals_file", std::string()));
    m_SingDurationSeconds = JsonLoader::Get(*json, "sing_duration", 4.0f) * 0.15f;

    if (json->contains("position") && (*json)["position"].is_array() && (*json)["position"].size() >= 2) {
        m_PosOffsetX = (*json)["position"][0].get<float>();
        m_PosOffsetY = (*json)["position"][1].get<float>();
    }
    if (json->contains("camera_position") && (*json)["camera_position"].is_array() && (*json)["camera_position"].size() >= 2) {
        m_CameraOffsetX = (*json)["camera_position"][0].get<float>();
        m_CameraOffsetY = (*json)["camera_position"][1].get<float>();
    }

    Dance();
    SetPosition(x, y);
    return true;
}

bool Character::LoadInternalGL(const std::string& characterId, bool keepConfiguredSprite) {
    const std::string charPath = Paths::CharacterData(characterId);
    if (charPath.empty()) {
        Logger::Warn("[Character] Character JSON not found: " + characterId);
        return false;
    }

    auto json = JsonLoader::LoadFile(charPath);
    if (!json.has_value()) {
        return false;
    }

    const std::string imageKey = JsonLoader::Get(*json, "image", std::string());
    if (imageKey.empty()) {
        Logger::Warn("[Character] Missing image key for: " + characterId);
        return false;
    }

    std::string imagePath = Paths::Image(imageKey);
    std::string xmlPath = Paths::Xml(imageKey);
    if (imagePath.empty()) {
        imagePath = Paths::Image(imageKey, "base_game");
        xmlPath = Paths::Xml(imageKey, "base_game");
    }
    if (imagePath.empty()) {
        Logger::Warn("[Character] Character atlas not found: " + imageKey);
        return false;
    }

    AnimatedSprite sprite;
    if (!sprite.LoadGL(imagePath, xmlPath)) {
        return false;
    }

    std::vector<AnimationDef> animations;
    if (json->contains("animations") && (*json)["animations"].is_array()) {
        for (const auto& anim : (*json)["animations"]) {
            if (!anim.is_object()) continue;

            AnimationDef def;
            def.anim = JsonLoader::Get(anim, "anim", std::string());
            def.name = JsonLoader::Get(anim, "name", std::string());
            def.fps = JsonLoader::Get(anim, "fps", 24);
            def.loop = JsonLoader::Get(anim, "loop", false);

            if (anim.contains("indices") && anim["indices"].is_array()) {
                for (const auto& index : anim["indices"]) {
                    def.indices.push_back(index.get<int>());
                }
            }

            if (anim.contains("offsets") && anim["offsets"].is_array() && anim["offsets"].size() >= 2) {
                def.offX = anim["offsets"][0].get<float>();
                def.offY = anim["offsets"][1].get<float>();
            }

            if (def.anim.empty() || def.name.empty()) continue;

            if (!def.indices.empty()) {
                sprite.AddByIndices(def.anim, def.name, def.indices, def.fps, def.loop);
            } else {
                sprite.AddByPrefix(def.anim, def.name, def.fps, def.loop);
            }

            animations.push_back(def);
        }
    }

    if (!keepConfiguredSprite) {
        return true;
    }

    m_Sprite = std::move(sprite);
    m_Animations = std::move(animations);
    m_Scale = JsonLoader::Get(*json, "scale", 1.0f);
    m_Sprite.SetScale(m_Scale);
    m_Sprite.flipX = JsonLoader::Get(*json, "flip_x", false);
    m_VocalsFile = JsonLoader::Get(*json, "vocalsFile", JsonLoader::Get(*json, "vocals_file", std::string()));
    m_SingDurationSeconds = JsonLoader::Get(*json, "sing_duration", 4.0f) * 0.15f;

    if (json->contains("position") && (*json)["position"].is_array() && (*json)["position"].size() >= 2) {
        m_PosOffsetX = (*json)["position"][0].get<float>();
        m_PosOffsetY = (*json)["position"][1].get<float>();
    }
    if (json->contains("camera_position") && (*json)["camera_position"].is_array() && (*json)["camera_position"].size() >= 2) {
        m_CameraOffsetX = (*json)["camera_position"][0].get<float>();
        m_CameraOffsetY = (*json)["camera_position"][1].get<float>();
    }

    Dance();
    SetPosition(x, y);
    return true;
}

bool Character::HasAnimation(const std::string& animName) const {
    for (const auto& anim : m_Animations) {
        if (anim.anim == animName) {
            return true;
        }
    }
    return false;
}

void Character::PlayAnimation(const std::string& animName, bool forceRestart) {
    for (const auto& anim : m_Animations) {
        if (anim.anim == animName) {
            m_CurrentAnimOffsetX = anim.offX;
            m_CurrentAnimOffsetY = anim.offY;
            m_Sprite.Play(animName, forceRestart);
            SetPosition(x, y);
            return;
        }
    }
}

void Character::SetPosition(float px, float py) {
    x = px;
    y = py;
    m_Sprite.x = x + m_PosOffsetX + m_CurrentAnimOffsetX;
    m_Sprite.y = y + m_PosOffsetY + m_CurrentAnimOffsetY;
}

void Character::Update(float dt) {
    if (m_HoldTimer > 0.0f) {
        m_HoldTimer -= dt;
        if (m_HoldTimer <= 0.0f) {
            m_HoldTimer = 0.0f;
            m_HoldLane = -1;
        }
    }

    m_Sprite.Update(dt);
}

void Character::Draw(SDL_Renderer* renderer) const {
    m_Sprite.Draw(renderer);
}

void Character::Draw(SDL_Renderer* renderer, float cameraX, float cameraY, float zoom) const {
    AnimatedSprite sprite = m_Sprite;
    sprite.x = (m_Sprite.x - cameraX) * zoom;
    sprite.y = (m_Sprite.y - cameraY) * zoom;
    sprite.scaleX = m_Sprite.scaleX * zoom;
    sprite.scaleY = m_Sprite.scaleY * zoom;
    sprite.Draw(renderer);
}

void Character::DrawGL(OpenGLESBackend& backend) const {
    m_Sprite.DrawGL(backend);
}

void Character::DrawGL(OpenGLESBackend& backend, float cameraX, float cameraY, float zoom) const {
    AnimatedSprite sprite = m_Sprite;
    sprite.x = (m_Sprite.x - cameraX) * zoom;
    sprite.y = (m_Sprite.y - cameraY) * zoom;
    sprite.scaleX = m_Sprite.scaleX * zoom;
    sprite.scaleY = m_Sprite.scaleY * zoom;
    sprite.DrawGL(backend);
}

void Character::Dance() {
    if (IsHolding()) {
        return;
    }

    if (HasAnimation("danceLeft") && HasAnimation("danceRight")) {
        m_DanceToggle = !m_DanceToggle;
        PlayAnimation(m_DanceToggle ? "danceRight" : "danceLeft", true);
        return;
    }

    if (HasAnimation("idle")) {
        PlayAnimation("idle", true);
    }
}

void Character::Sing(int lane, float holdDuration) {
    static const char* kAnimNames[4] = { "singLEFT", "singDOWN", "singUP", "singRIGHT" };
    if (lane < 0 || lane > 3) {
        return;
    }

    if (HasAnimation(kAnimNames[lane])) {
        PlayAnimation(kAnimNames[lane], true);
        if (holdDuration > 0.0f) {
            m_HoldLane = lane;
            m_HoldTimer = holdDuration;
        }
    }
}

void Character::Hold(int lane, float duration) {
    Sing(lane, duration);
}

void Character::StopHold(int lane) {
    if (lane < 0 || m_HoldLane == lane) {
        m_HoldLane = -1;
        m_HoldTimer = 0.0f;
    }
}

SDL_FPoint Character::GetCameraFocusPoint() const {
    SDL_FPoint point = {
        x + m_PosOffsetX + m_CurrentAnimOffsetX + m_Sprite.GetWidth() * 0.5f + m_CameraOffsetX,
        y + m_PosOffsetY + m_CurrentAnimOffsetY + m_Sprite.GetHeight() * 0.35f + m_CameraOffsetY
    };
    return point;
}

} // namespace FNF
