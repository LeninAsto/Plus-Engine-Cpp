/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * SoundPlayer - Short sound effects via SDL_mixer chunks
 */

#pragma once

#include <SDL2/SDL_mixer.h>
#include <string>
#include <unordered_map>

namespace FNF {

class SoundPlayer {
public:
    static bool Play(const std::string& path, float volume = 1.0f);
    static void Shutdown();

private:
    static Mix_Chunk* Load(const std::string& path);

    static std::unordered_map<std::string, Mix_Chunk*> s_Cache;
};

} // namespace FNF