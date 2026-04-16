/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Sprite Implementation
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#include "Sprite.h"
#include "Texture.h"
#include "../core/Logger.h"

namespace FNF {

bool Sprite::Load(SDL_Renderer* renderer, const std::string& path) {
    m_Texture = TextureCache::Load(renderer, path);
    if (!m_Texture) return false;

    SDL_QueryTexture(m_Texture, nullptr, nullptr, &texWidth, &texHeight);
    return true;
}

void Sprite::Draw(SDL_Renderer* renderer) const {
    if (!visible || !m_Texture) return;

    // Apply alpha
    SDL_SetTextureAlphaMod(m_Texture, static_cast<Uint8>(alpha * 255.0f));

    // Apply color tint
    SDL_SetTextureColorMod(m_Texture, colorR, colorG, colorB);

    // Enable blending for alpha support
    SDL_SetTextureBlendMode(m_Texture, SDL_BLENDMODE_BLEND);

    // Destination rect
    SDL_Rect dst = {
        static_cast<int>(x),
        static_cast<int>(y),
        static_cast<int>(texWidth  * scaleX),
        static_cast<int>(texHeight * scaleY)
    };

    if (angle != 0.0f) {
        SDL_RenderCopyEx(renderer, m_Texture, nullptr, &dst,
                         static_cast<double>(angle), nullptr, SDL_FLIP_NONE);
    } else {
        SDL_RenderCopy(renderer, m_Texture, nullptr, &dst);
    }
}

void Sprite::ScreenCenter(int screenW, int screenH) {
    x = (screenW  - GetWidth())  * 0.5f;
    y = (screenH - GetHeight()) * 0.5f;
}

} // namespace FNF
