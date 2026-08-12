/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Texture Cache Implementation
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#include "Texture.h"
#include "../backend/Logger.h"
#include "../backend/OpenGLESBackend.h"

#include <algorithm>

namespace FNF {

std::unordered_map<std::string, SDL_Texture*> TextureCache::s_Cache;
std::unordered_map<std::string, TextureHandle> TextureCache::s_GLCache;
OpenGLESBackend* TextureCache::s_GLBackend = nullptr;
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

    SDL_Surface* surface = LoadSurface(path);
    if (!surface) {
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

SDL_Texture* TextureCache::LoadPaletteMapped(SDL_Renderer* renderer,
                                             const std::string& path,
                                             const std::string& cacheKey,
                                             SDL_Color redChannel,
                                             SDL_Color greenChannel,
                                             SDL_Color blueChannel,
                                             float mult) {
    auto it = s_Cache.find(cacheKey);
    if (it != s_Cache.end()) {
        return it->second;
    }

    SDL_Surface* surface = LoadPaletteMappedSurface(path, redChannel, greenChannel, blueChannel, mult);
    if (!surface) {
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        Logger::Error("TextureCache: SDL_CreateTextureFromSurface failed for palette map: " + std::string(SDL_GetError()));
        return nullptr;
    }

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    s_Cache[cacheKey] = texture;
    return texture;
}

void TextureCache::BindOpenGLESBackend(OpenGLESBackend* backend) {
    s_GLBackend = backend;
}

TextureHandle TextureCache::LoadGL(const std::string& path) {
    auto it = s_GLCache.find(path);
    if (it != s_GLCache.end()) {
        return it->second;
    }

    if (!s_GLBackend) {
        Logger::Error("TextureCache: OpenGLESBackend is not bound");
        return {};
    }

    SDL_Surface* surface = LoadSurface(path);
    if (!surface) return {};

    TextureHandle texture = s_GLBackend->CreateTextureFromSurface(surface, path);
    SDL_FreeSurface(surface);

    if (!texture.IsValid()) return {};
    s_GLCache[path] = texture;
    return texture;
}

TextureHandle TextureCache::LoadPaletteMappedGL(const std::string& path,
                                                const std::string& cacheKey,
                                                SDL_Color redChannel,
                                                SDL_Color greenChannel,
                                                SDL_Color blueChannel,
                                                float mult) {
    auto it = s_GLCache.find(cacheKey);
    if (it != s_GLCache.end()) {
        return it->second;
    }

    if (!s_GLBackend) {
        Logger::Error("TextureCache: OpenGLESBackend is not bound");
        return {};
    }

    SDL_Surface* surface = LoadPaletteMappedSurface(path, redChannel, greenChannel, blueChannel, mult);
    if (!surface) return {};

    TextureHandle texture = s_GLBackend->CreateTextureFromSurface(surface, cacheKey);
    SDL_FreeSurface(surface);

    if (!texture.IsValid()) return {};
    s_GLCache[cacheKey] = texture;
    return texture;
}

void TextureCache::Unload(const std::string& path) {
    auto it = s_Cache.find(path);
    if (it != s_Cache.end()) {
        SDL_DestroyTexture(it->second);
        s_Cache.erase(it);
    }

    auto glIt = s_GLCache.find(path);
    if (glIt != s_GLCache.end()) {
        if (s_GLBackend) {
            s_GLBackend->DestroyTexture(glIt->second);
        }
        s_GLCache.erase(glIt);
    }
}

void TextureCache::ClearAll() {
    for (auto& [path, tex] : s_Cache) {
        SDL_DestroyTexture(tex);
    }
    s_Cache.clear();

    if (s_GLBackend) {
        for (auto& [path, tex] : s_GLCache) {
            s_GLBackend->DestroyTexture(tex);
        }
    }
    s_GLCache.clear();
}

SDL_Surface* TextureCache::LoadSurface(const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        Logger::Error("TextureCache: failed to load '" + path + "': " + IMG_GetError());
        return nullptr;
    }
    return surface;
}

SDL_Surface* TextureCache::LoadPaletteMappedSurface(const std::string& path,
                                                    SDL_Color redChannel,
                                                    SDL_Color greenChannel,
                                                    SDL_Color blueChannel,
                                                    float mult) {
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (!loadedSurface) {
        Logger::Error("TextureCache: failed to load palette source '" + path + "': " + IMG_GetError());
        return nullptr;
    }

    SDL_Surface* surface = SDL_ConvertSurfaceFormat(loadedSurface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(loadedSurface);
    if (!surface) {
        Logger::Error("TextureCache: SDL_ConvertSurfaceFormat failed: " + std::string(SDL_GetError()));
        return nullptr;
    }

    mult = (std::clamp)(mult, 0.0f, 1.0f);

    auto scaleChannel = [](Uint8 srcR, Uint8 srcG, Uint8 srcB, Uint8 redValue, Uint8 greenValue, Uint8 blueValue) -> Uint8 {
        const int mapped = (static_cast<int>(srcR) * static_cast<int>(redValue)
                          + static_cast<int>(srcG) * static_cast<int>(greenValue)
                          + static_cast<int>(srcB) * static_cast<int>(blueValue)) / 255;
        return static_cast<Uint8>((std::clamp)(mapped, 0, 255));
    };

    Uint32* pixels = static_cast<Uint32*>(surface->pixels);
    const int pixelCount = surface->w * surface->h;
    for (int i = 0; i < pixelCount; ++i) {
        Uint8 srcR = 0;
        Uint8 srcG = 0;
        Uint8 srcB = 0;
        Uint8 srcA = 0;
        SDL_GetRGBA(pixels[i], surface->format, &srcR, &srcG, &srcB, &srcA);

        const Uint8 mappedR = scaleChannel(srcR, srcG, srcB, redChannel.r, greenChannel.r, blueChannel.r);
        const Uint8 mappedG = scaleChannel(srcR, srcG, srcB, redChannel.g, greenChannel.g, blueChannel.g);
        const Uint8 mappedB = scaleChannel(srcR, srcG, srcB, redChannel.b, greenChannel.b, blueChannel.b);

        const Uint8 outR = static_cast<Uint8>(srcR + static_cast<int>((static_cast<float>(mappedR) - srcR) * mult));
        const Uint8 outG = static_cast<Uint8>(srcG + static_cast<int>((static_cast<float>(mappedG) - srcG) * mult));
        const Uint8 outB = static_cast<Uint8>(srcB + static_cast<int>((static_cast<float>(mappedB) - srcB) * mult));
        pixels[i] = SDL_MapRGBA(surface->format, outR, outG, outB, srcA);
    }

    return surface;
}

} // namespace FNF
