/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Stage implementation.
 */

#include "Stage.h"
#include "../core/Logger.h"
#include "../data/JsonLoader.h"
#include "../data/Paths.h"
#include "../graphics/Texture.h"

namespace {

std::string StripImagesPrefix(const std::string& key) {
    if (key.rfind("images/", 0) == 0) {
        return key.substr(7);
    }
    return key;
}

} // namespace

namespace FNF {

std::optional<StageData> StageScene::LoadData(const std::string& stageName) {
    const std::string stagePath = Paths::StageData(stageName);
    if (stagePath.empty()) {
        Logger::Warn("[StageScene] Stage JSON not found: " + stageName);
        return std::nullopt;
    }

    auto json = JsonLoader::LoadFile(stagePath);
    if (!json.has_value()) {
        return std::nullopt;
    }

    StageData out;
    out.stageName = stageName;
    out.directory = JsonLoader::Get(*json, "directory", std::string("week1"));
    out.defaultZoom = JsonLoader::Get(*json, "defaultZoom", 0.9f);
    out.hideGirlfriend = JsonLoader::Get(*json, "hide_girlfriend", false);

    if (json->contains("boyfriend") && (*json)["boyfriend"].is_array() && (*json)["boyfriend"].size() >= 2) {
        out.boyfriendX = (*json)["boyfriend"][0].get<float>();
        out.boyfriendY = (*json)["boyfriend"][1].get<float>();
    }
    if (json->contains("opponent") && (*json)["opponent"].is_array() && (*json)["opponent"].size() >= 2) {
        out.opponentX = (*json)["opponent"][0].get<float>();
        out.opponentY = (*json)["opponent"][1].get<float>();
    }
    if (json->contains("girlfriend") && (*json)["girlfriend"].is_array() && (*json)["girlfriend"].size() >= 2) {
        out.girlfriendX = (*json)["girlfriend"][0].get<float>();
        out.girlfriendY = (*json)["girlfriend"][1].get<float>();
    }

    if (json->contains("preload") && (*json)["preload"].is_object()) {
        for (auto it = (*json)["preload"].begin(); it != (*json)["preload"].end(); ++it) {
            out.preloadImages.push_back(StripImagesPrefix(it.key()));
        }
    }

    return out;
}

bool StageScene::Precache(SDL_Renderer* renderer, const StageData& data) {
    for (const auto& key : data.preloadImages) {
        std::string imagePath = Paths::Image(key, data.directory);
        if (imagePath.empty()) {
            imagePath = Paths::Image(key);
        }
        if (!imagePath.empty()) {
            TextureCache::Load(renderer, imagePath);
        }
    }
    return true;
}

bool StageScene::Load(SDL_Renderer* renderer, const StageData& data) {
    std::string backPath = Paths::Image("stageback", data.directory);
    std::string frontPath = Paths::Image("stagefront", data.directory);
    std::string curtainsPath = Paths::Image("stagecurtains", data.directory);

    if (!backPath.empty()) {
        m_Back.Load(renderer, backPath);
        m_Back.x = -600.0f;
        m_Back.y = -200.0f;
    }
    if (!frontPath.empty()) {
        m_Front.Load(renderer, frontPath);
        m_Front.x = -650.0f;
        m_Front.y = 600.0f;
        m_Front.scaleX = 1.1f;
        m_Front.scaleY = 1.1f;
    }
    if (!curtainsPath.empty()) {
        m_Curtains.Load(renderer, curtainsPath);
        m_Curtains.x = -500.0f;
        m_Curtains.y = -300.0f;
        m_Curtains.scaleX = 0.9f;
        m_Curtains.scaleY = 0.9f;
    }

    return m_Back.IsLoaded() || m_Front.IsLoaded() || m_Curtains.IsLoaded();
}

void StageScene::Draw(SDL_Renderer* renderer) const {
    m_Back.Draw(renderer);
    m_Front.Draw(renderer);
    m_Curtains.Draw(renderer);
}

} // namespace FNF