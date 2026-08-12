/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Asset Path Manager Implementation
 *
 * Author: LeninAsto
 * Date: March 2026
 */

#include "Paths.h"
#include "../backend/Logger.h"
#include <algorithm>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace {

std::filesystem::path GetExecutableDirectory() {
#ifdef _WIN32
    wchar_t modulePath[MAX_PATH] = {};
    const DWORD pathLength = GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
    if (pathLength > 0 && pathLength < MAX_PATH) {
        return std::filesystem::path(modulePath).parent_path();
    }
#endif
    return {};
}

bool TryResolveAssetsRoot(const std::filesystem::path& startPath,
                          const std::string& assetsRoot,
                          std::string& resolvedRoot) {
    namespace fs = std::filesystem;

    if (startPath.empty()) {
        return false;
    }

    fs::path search = startPath;
    for (int i = 0; i < 8; ++i) {
        fs::path candidate = search / assetsRoot;
        if (fs::exists(candidate)) {
            resolvedRoot = candidate.generic_string();
            return true;
        }

        fs::path parent = search.parent_path();
        if (parent == search) {
            break;
        }
        search = parent;
    }

    return false;
}

} // namespace

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

    const std::filesystem::path configuredRoot = s_AssetsRoot;
    if (configuredRoot.is_absolute() && std::filesystem::exists(configuredRoot)) {
        s_AssetsRoot = configuredRoot.generic_string();
        Logger::Info("[OK] Paths initialized - root: " + s_AssetsRoot);
        return;
    }

    std::string resolvedRoot;

    const std::filesystem::path exeDir = GetExecutableDirectory();
    if (TryResolveAssetsRoot(exeDir, assetsRoot, resolvedRoot)) {
        s_AssetsRoot = resolvedRoot;
        Logger::Info("[OK] Paths initialized - root: " + s_AssetsRoot);
        return;
    }

    if (std::filesystem::exists(configuredRoot)) {
        s_AssetsRoot = configuredRoot.generic_string();
        Logger::Info("[OK] Paths initialized - root: " + s_AssetsRoot);
        return;
    }

    // Walk up the directory tree looking for the assets folder.
    // This handles running the exe from build-cpp/bin/Debug/ while
    // assets live at the project root.
    namespace fs = std::filesystem;
    if (TryResolveAssetsRoot(fs::current_path(), assetsRoot, resolvedRoot)) {
        s_AssetsRoot = resolvedRoot;
        Logger::Info("[OK] Paths initialized - root: " + s_AssetsRoot);
        return;
    }

    Logger::Warn("Assets root not found after directory search: " + assetsRoot);
}

// =========================================================================
// Audio
// =========================================================================

std::string Paths::Inst(const std::string& songName) {
    return ResolveFirstExisting({
        ResolveAudio("songs/" + songName + "/Inst"),
        ResolveAudio("base_game/songs/" + songName + "/Inst")
    });
}

std::string Paths::Voices(const std::string& songName, const std::string& suffix) {
    std::string file = "Voices" + (suffix.empty() ? "" : "-" + suffix);
    return ResolveFirstExisting({
        ResolveAudio("songs/" + songName + "/" + file),
        ResolveAudio("base_game/songs/" + songName + "/" + file)
    });
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
    if (library == "shared" || library == "base_game") {
        return ResolveFirstExisting({
            "shared/images/" + key + ".png",
            "base_game/shared/images/" + key + ".png",
            "base_game/images/" + key + ".png"
        });
    }

    return ResolveFirstExisting({
        library + "/images/" + key + ".png",
        "base_game/" + library + "/images/" + key + ".png"
    });
}

std::string Paths::Xml(const std::string& key, const std::string& library) {
    if (library == "shared" || library == "base_game") {
        return ResolveFirstExisting({
            "shared/images/" + key + ".xml",
            "base_game/shared/images/" + key + ".xml",
            "base_game/images/" + key + ".xml"
        });
    }

    return ResolveFirstExisting({
        library + "/images/" + key + ".xml",
        "base_game/" + library + "/images/" + key + ".xml"
    });
}

std::string Paths::ImageJson(const std::string& key, const std::string& library) {
    if (library == "shared" || library == "base_game") {
        return ResolveFirstExisting({
            "shared/images/" + key + ".json",
            "base_game/shared/images/" + key + ".json",
            "base_game/images/" + key + ".json"
        });
    }

    return ResolveFirstExisting({
        library + "/images/" + key + ".json",
        "base_game/" + library + "/images/" + key + ".json"
    });
}

// =========================================================================
// Data / Charts
// =========================================================================

std::string Paths::SongData(const std::string& songName, const std::string& difficulty) {
    std::string fileName = songName;
    if (!difficulty.empty()) {
        fileName += "-" + difficulty;
    }
    return SongVariantData(songName, fileName);
}

std::string Paths::SongMeta(const std::string& songName) {
    return ResolveFirstExisting({
        "shared/data/" + songName + "/metadata.json",
        "base_game/shared/data/" + songName + "/metadata.json"
    });
}

std::string Paths::CharacterData(const std::string& charName) {
    return ResolveFirstExisting({
        "shared/characters/" + charName + ".json",
        "base_game/shared/characters/" + charName + ".json"
    });
}

std::string Paths::StageData(const std::string& stageName) {
    return ResolveFirstExisting({
        "shared/stages/" + stageName + ".json",
        "base_game/shared/stages/" + stageName + ".json"
    });
}

std::string Paths::WeekData(const std::string& weekName) {
    return ResolveFirstExisting({
        "shared/weeks/" + weekName + ".json",
        "base_game/shared/weeks/" + weekName + ".json"
    });
}

std::string Paths::SongVariantData(const std::string& songFolder, const std::string& fileStem) {
    return ResolveFirstExisting({
        "shared/data/" + songFolder + "/" + fileStem + ".json",
        "base_game/shared/data/" + songFolder + "/" + fileStem + ".json",
        "base_game/data/" + songFolder + "/" + fileStem + ".json"
    });
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

std::string Paths::ResolveFirstExisting(const std::vector<std::string>& relatives) {
    for (const auto& relative : relatives) {
        if (relative.empty()) {
            continue;
        }

        if (relative.find('/') != std::string::npos && std::filesystem::exists(relative)) {
            return relative;
        }

        std::string full = ResolvePath(relative);
        if (!full.empty()) {
            return full;
        }
    }

    return "";
}

} // namespace FNF
