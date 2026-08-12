/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Sprite - Basic 2D Sprite
 * 
 * Wraps an SDL_Texture with position, scale, alpha, and tint.
 * No animation atlas yet - just static image rendering.
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#pragma once

#include <SDL2/SDL.h>
#include <string>

#include "../backend/Basic.h"
#include "../backend/RenderTypes.h"
#include "Transform2D.h"

namespace FNF {

class OpenGLESBackend;

class Sprite : public Basic, public Transform2D {
public:
    // Color tint (default white = no tint)
    Uint8 colorR = 255;
    Uint8 colorG = 255;
    Uint8 colorB = 255;

    // Actual pixel dimensions of the loaded texture
    int texWidth  = 0;
    int texHeight = 0;

    /**
     * Load a texture from disk (uses TextureCache internally).
     * @return true on success
     */
    bool Load(SDL_Renderer* renderer, const std::string& path);
    bool LoadGL(const std::string& path);

    /**
     * Render the sprite using the given renderer.
     */
    void Draw(SDL_Renderer* renderer) const override;
    void Draw(SDL_Renderer* renderer, float offsetX, float offsetY, float zoom = 1.0f) const;
    void DrawGL(OpenGLESBackend& backend) const;
    void DrawGL(OpenGLESBackend& backend, float offsetX, float offsetY, float zoom = 1.0f) const;

    /**
     * Center this sprite on the screen (1280x720).
     * Optionally pass the window dimensions.
     */
    void ScreenCenter(int screenW = 1280, int screenH = 720);

    /** Returns the scaled display width */
    float GetWidth()  const { return texWidth  * scaleX; }
    float GetHeight() const { return texHeight * scaleY; }

    bool IsLoaded() const { return m_Texture != nullptr || m_GLTexture.IsValid(); }

private:
    SDL_Texture* m_Texture = nullptr;
    TextureHandle m_GLTexture;
};

} // namespace FNF
