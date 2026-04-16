/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * SoundPlayer Implementation
 */

#include "SoundPlayer.h"
#include "../core/Logger.h"
#include <algorithm>

namespace FNF {

std::unordered_map<std::string, Mix_Chunk*> SoundPlayer::s_Cache;

Mix_Chunk* SoundPlayer::Load(const std::string& path) {
    auto it = s_Cache.find(path);
    if (it != s_Cache.end()) {
        return it->second;
    }

    Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
    if (!chunk) {
        Logger::Warn("[SoundPlayer] Failed to load sound: " + path + " -> " + Mix_GetError());
        return nullptr;
    }

    s_Cache[path] = chunk;
    return chunk;
}

bool SoundPlayer::Play(const std::string& path, float volume) {
    Mix_Chunk* chunk = Load(path);
    if (!chunk) {
        return false;
    }

    volume = (std::clamp)(volume, 0.0f, 1.0f);
    Mix_VolumeChunk(chunk, static_cast<int>(volume * MIX_MAX_VOLUME));

    if (Mix_PlayChannel(-1, chunk, 0) < 0) {
        Logger::Warn("[SoundPlayer] Failed to play sound: " + path + " -> " + Mix_GetError());
        return false;
    }

    return true;
}

void SoundPlayer::ClearCache() {
    Mix_HaltChannel(-1);

    for (auto& entry : s_Cache) {
        if (entry.second) {
            Mix_FreeChunk(entry.second);
        }
    }
    s_Cache.clear();
}

void SoundPlayer::Shutdown() {
    ClearCache();
}

} // namespace FNF