#include "VocalsPlayer.h"

#include "../backend/Logger.h"

#include <algorithm>

namespace FNF {

std::unordered_map<std::string, Mix_Chunk*> VocalsPlayer::s_Cache;
float VocalsPlayer::s_Volume = 1.0f;

Mix_Chunk* VocalsPlayer::Load(const std::string& path) {
    if (path.empty()) {
        return nullptr;
    }

    auto it = s_Cache.find(path);
    if (it != s_Cache.end()) {
        return it->second;
    }

    Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
    if (!chunk) {
        Logger::Warn("[VocalsPlayer] Failed to load vocals: " + path + " -> " + Mix_GetError());
        return nullptr;
    }

    s_Cache[path] = chunk;
    return chunk;
}

bool VocalsPlayer::PreloadPlayer(const std::string& path) {
    return Load(path) != nullptr;
}

bool VocalsPlayer::PreloadOpponent(const std::string& path) {
    return Load(path) != nullptr;
}

bool VocalsPlayer::Play(const std::string& playerPath, const std::string& opponentPath, float volume) {
    Stop();
    SetVolume(volume);

    bool anyStarted = false;

    if (Mix_Chunk* player = Load(playerPath)) {
        if (Mix_PlayChannel(kPlayerChannel, player, 0) >= 0) {
            Mix_Volume(kPlayerChannel, static_cast<int>(s_Volume * MIX_MAX_VOLUME));
            anyStarted = true;
        }
    }

    if (Mix_Chunk* opponent = Load(opponentPath)) {
        if (Mix_PlayChannel(kOpponentChannel, opponent, 0) >= 0) {
            Mix_Volume(kOpponentChannel, static_cast<int>(s_Volume * MIX_MAX_VOLUME));
            anyStarted = true;
        }
    }

    return anyStarted;
}

void VocalsPlayer::Stop() {
    Mix_HaltChannel(kPlayerChannel);
    Mix_HaltChannel(kOpponentChannel);
}

void VocalsPlayer::Pause() {
    Mix_Pause(kPlayerChannel);
    Mix_Pause(kOpponentChannel);
}

void VocalsPlayer::Resume() {
    Mix_Resume(kPlayerChannel);
    Mix_Resume(kOpponentChannel);
}

void VocalsPlayer::SetVolume(float volume) {
    s_Volume = (std::clamp)(volume, 0.0f, 1.0f);
    Mix_Volume(kPlayerChannel, static_cast<int>(s_Volume * MIX_MAX_VOLUME));
    Mix_Volume(kOpponentChannel, static_cast<int>(s_Volume * MIX_MAX_VOLUME));
}

void VocalsPlayer::ClearCache() {
    Stop();

    for (auto& entry : s_Cache) {
        if (entry.second) {
            Mix_FreeChunk(entry.second);
        }
    }
    s_Cache.clear();
}

} // namespace FNF