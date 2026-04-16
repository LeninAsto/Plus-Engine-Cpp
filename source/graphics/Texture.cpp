/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Texture Cache Implementation
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#include "Texture.h"
#include "../core/Logger.h"

namespace FNF {

std::unordered_map<std::string, SDL_Texture*> TextureCache::s_Cache;
bool TextureCache::s_Initialized = false;

bool TextureCache::Init() {
    if (s_Initialized) return true;

    int flags = IMG_INIT_PNG;
    int inited = IMG_Init(flags);
    if ((inited & flags) != flags) {
        Logger::Error("TextureCache: IMG_Init failed: " + std::string(IMG_GetError()));
        return false;
    }

    s_Initialized = true;
    Logger::Info("[OK] TextureCache initialized (PNG support)");
    return true;
}

void TextureCache::Shutdown() {
    ClearAll();
    IMG_Quit();
    s_Initialized = false;
}

SDL_Texture* TextureCache::Load(SDL_Renderer* renderer, const std::string& path) {
    // Return cached texture if available
    auto it = s_Cache.find(path);
    if (it != s_Cache.end()) {
        return it->second;
    }

    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        Logger::Error("TextureCache: failed to load '" + path + "': " + IMG_GetError());
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        Logger::Error("TextureCache: SDL_CreateTextureFromSurface failed: " + std::string(SDL_GetError()));
        return nullptr;
    }

    s_Cache[path] = texture;
    return texture;
}

void TextureCache::Unload(const std::string& path) {
    auto it = s_Cache.find(path);
    if (it != s_Cache.end()) {
        SDL_DestroyTexture(it->second);
        s_Cache.erase(it);
    }
}

void TextureCache::ClearAll() {
    for (auto& [path, tex] : s_Cache) {
        SDL_DestroyTexture(tex);
    }
    s_Cache.clear();
}

} // namespace FNF
