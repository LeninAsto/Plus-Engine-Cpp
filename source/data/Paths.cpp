/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Asset Path Manager Implementation
 *
 * Author: LeninAsto
 * Date: March 2026
 */

#include "Paths.h"
#include "../core/Logger.h"
#include <algorithm>

namespace FNF {

// Static initialization
std::string Paths::s_AssetsRoot = "assets";

void Paths::Init(const std::string& assetsRoot) {
    s_AssetsRoot = assetsRoot;

    // Normalize slashes
    std::replace(s_AssetsRoot.begin(), s_AssetsRoot.end(), '\\', '/');

    // Strip trailing slash
    if (!s_AssetsRoot.empty() && s_AssetsRoot.back() == '/') {
        s_AssetsRoot.pop_back();
    }

    // If the given path already exists, we're done
    if (std::filesystem::exists(s_AssetsRoot)) {
        Logger::Info("[OK] Paths initialized - root: " + s_AssetsRoot);
        return;
    }

    // Walk up the directory tree looking for the assets folder.
    // This handles running the exe from build-cpp/bin/Debug/ while
    // assets live at the project root.
    namespace fs = std::filesystem;
    fs::path search = fs::current_path();
    for (int i = 0; i < 8; ++i) {
        fs::path candidate = search / assetsRoot;
        if (fs::exists(candidate)) {
            s_AssetsRoot = candidate.generic_string();
            Logger::Info("[OK] Paths initialized - root: " + s_AssetsRoot);
            return;
        }
        fs::path parent = search.parent_path();
        if (parent == search) break;  // reached filesystem root
        search = parent;
    }

    Logger::Warn("Assets root not found after directory search: " + assetsRoot);
}

// =========================================================================
// Audio
// =========================================================================

std::string Paths::Inst(const std::string& songName) {
    // base_game/songs/<name>/Inst
    return ResolveAudio("base_game/songs/" + songName + "/Inst");
}

std::string Paths::Voices(const std::string& songName, const std::string& suffix) {
    std::string file = "Voices" + (suffix.empty() ? "" : "-" + suffix);
    return ResolveAudio("base_game/songs/" + songName + "/" + file);
}

std::string Paths::Sound(const std::string& key) {
    return ResolveAudio("shared/sounds/" + key);
}

std::string Paths::Music(const std::string& key) {
    return ResolveAudio("shared/music/" + key);
}

// =========================================================================
// Images
// =========================================================================

std::string Paths::Image(const std::string& key, const std::string& library) {
    return ResolvePath(library + "/images/" + key + ".png");
}

std::string Paths::Xml(const std::string& key, const std::string& library) {
    return ResolvePath(library + "/images/" + key + ".xml");
}

std::string Paths::ImageJson(const std::string& key, const std::string& library) {
    return ResolvePath(library + "/images/" + key + ".json");
}

// =========================================================================
// Data / Charts
// =========================================================================

std::string Paths::SongData(const std::string& songName, const std::string& difficulty) {
    std::string fileName = songName;
    if (!difficulty.empty()) {
        fileName += "-" + difficulty;
    }
    // New format: base_game/shared/data/<song>/<song>.json
    std::string newPath = ResolvePath("base_game/shared/data/" + songName + "/" + fileName + ".json");
    if (!newPath.empty()) return newPath;

    // Legacy fallback
    return ResolvePath("base_game/data/" + songName + "/" + fileName + ".json");
}

std::string Paths::SongMeta(const std::string& songName) {
    return ResolvePath("base_game/shared/data/" + songName + "/metadata.json");
}

std::string Paths::CharacterData(const std::string& charName) {
    return ResolvePath("shared/characters/" + charName + ".json");
}

std::string Paths::StageData(const std::string& stageName) {
    return ResolvePath("shared/stages/" + stageName + ".json");
}

// =========================================================================
// Fonts
// =========================================================================

std::string Paths::Font(const std::string& key) {
    return ResolvePath("fonts/" + key);
}

// =========================================================================
// Helpers
// =========================================================================

bool Paths::Exists(const std::string& path) {
    return std::filesystem::exists(path);
}

std::string Paths::ResolvePath(const std::string& relative) {
    std::string full = s_AssetsRoot + "/" + relative;
    if (std::filesystem::exists(full)) {
        return full;
    }
    return "";
}

std::string Paths::ResolveAudio(const std::string& relative) {
    // Prefer .ogg, fall back to .mp3
    std::string ogg = ResolvePath(relative + ".ogg");
    if (!ogg.empty()) return ogg;

    std::string mp3 = ResolvePath(relative + ".mp3");
    if (!mp3.empty()) return mp3;

    return "";
}

} // namespace FNF
