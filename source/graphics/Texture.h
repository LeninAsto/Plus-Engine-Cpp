/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Texture Cache
 * 
 * Loads PNG/JPG images via SDL2_image and caches them by path.
 * Sprites reference textures from this cache.
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <unordered_map>

namespace FNF {

class TextureCache {
public:
    static bool Init();  // Initializes SDL2_image with PNG support
    static void Shutdown();

    /**
     * Load a texture from disk (or return the cached one).
     * Returns nullptr on failure.
     */
    static SDL_Texture* Load(SDL_Renderer* renderer, const std::string& path);
    static SDL_Texture* LoadPaletteMapped(SDL_Renderer* renderer,
                                          const std::string& path,
                                          const std::string& cacheKey,
                                          SDL_Color redChannel,
                                          SDL_Color greenChannel,
                                          SDL_Color blueChannel,
                                          float mult = 1.0f);

    /**
     * Free a specific texture from the cache.
     */
    static void Unload(const std::string& path);

    /**
     * Free ALL cached textures (call between states).
     */
    static void ClearAll();

    static int CachedCount() { return static_cast<int>(s_Cache.size()); }

private:
    static std::unordered_map<std::string, SDL_Texture*> s_Cache;
    static bool s_Initialized;
};

} // namespace FNF
