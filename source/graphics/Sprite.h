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

namespace FNF {

class Sprite {
public:
    float x       = 0.0f;
    float y       = 0.0f;
    float scaleX  = 1.0f;
    float scaleY  = 1.0f;
    float alpha   = 1.0f;   // 0.0 to 1.0
    float angle   = 0.0f;   // degrees
    bool  visible = true;
    bool  flipX   = false;
    bool  flipY   = false;

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

    /**
     * Render the sprite using the given renderer.
     */
    void Draw(SDL_Renderer* renderer) const;
    void Draw(SDL_Renderer* renderer, float offsetX, float offsetY, float zoom = 1.0f) const;

    /**
     * Center this sprite on the screen (1280x720).
     * Optionally pass the window dimensions.
     */
    void ScreenCenter(int screenW = 1280, int screenH = 720);

    /** Convenience: set uniform scale */
    void SetScale(float s) { scaleX = scaleY = s; }

    /** Returns the scaled display width */
    float GetWidth()  const { return texWidth  * scaleX; }
    float GetHeight() const { return texHeight * scaleY; }

    bool IsLoaded() const { return m_Texture != nullptr; }

private:
    SDL_Texture* m_Texture = nullptr;
};

} // namespace FNF
