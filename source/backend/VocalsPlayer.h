#pragma once

#include <SDL2/SDL_mixer.h>
#include <string>
#include <unordered_map>

namespace FNF {

class VocalsPlayer {
public:
    static bool PreloadPlayer(const std::string& path);
    static bool PreloadOpponent(const std::string& path);
    static bool Play(const std::string& playerPath, const std::string& opponentPath, float volume = 1.0f);
    static void Stop();
    static void Pause();
    static void Resume();
    static void SetVolume(float volume);
    static void ClearCache();

private:
    static Mix_Chunk* Load(const std::string& path);

    static constexpr int kPlayerChannel = 30;
    static constexpr int kOpponentChannel = 31;

    static std::unordered_map<std::string, Mix_Chunk*> s_Cache;
    static float s_Volume;
};

} // namespace FNF