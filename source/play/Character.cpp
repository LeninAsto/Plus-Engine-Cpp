/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Character implementation.
 */

#include "Character.h"
#include "../core/Logger.h"
#include "../data/JsonLoader.h"
#include "../data/Paths.h"

namespace FNF {

bool Character::Load(SDL_Renderer* renderer, const std::string& characterId) {
    return LoadInternal(renderer, characterId, true);
}

bool Character::Precache(SDL_Renderer* renderer, const std::string& characterId) {
    return LoadInternal(renderer, characterId, false);
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

    if (json->contains("position") && (*json)["position"].is_array() && (*json)["position"].size() >= 2) {
        m_PosOffsetX = (*json)["position"][0].get<float>();
        m_PosOffsetY = (*json)["position"][1].get<float>();
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
    m_Sprite.Update(dt);
}

void Character::Draw(SDL_Renderer* renderer) const {
    m_Sprite.Draw(renderer);
}

void Character::Dance() {
    if (HasAnimation("danceLeft") && HasAnimation("danceRight")) {
        m_DanceToggle = !m_DanceToggle;
        PlayAnimation(m_DanceToggle ? "danceRight" : "danceLeft", true);
        return;
    }

    if (HasAnimation("idle")) {
        PlayAnimation("idle", true);
    }
}

void Character::Sing(int lane) {
    static const char* kAnimNames[4] = { "singLEFT", "singDOWN", "singUP", "singRIGHT" };
    if (lane < 0 || lane > 3) {
        return;
    }

    if (HasAnimation(kAnimNames[lane])) {
        PlayAnimation(kAnimNames[lane], true);
    }
}

} // namespace FNF