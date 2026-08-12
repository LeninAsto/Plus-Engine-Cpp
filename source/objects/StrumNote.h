#pragma once

#include "../objects/AnimatedSprite.h"

#include <SDL2/SDL.h>

namespace FNF {

class OpenGLESBackend;

class StrumNote {
public:
    static constexpr float kScale = 0.7f;

    bool Load(SDL_Renderer* renderer, int lane, bool player);
    bool LoadGL(int lane, bool player);

    void SetPosition(float x, float y);
    void Update(float dt);
    void Draw(SDL_Renderer* renderer) const;
    void DrawGL(OpenGLESBackend& backend) const;

    void Press();
    void Release();
    void Confirm(float duration = 0.12f);

    float GetX() const { return m_LogicalX; }
    float GetY() const { return m_LogicalY; }
    float GetWidth() const { return m_LogicalWidth; }
    float GetHeight() const { return m_LogicalHeight; }
    float GetCenterX() const { return m_LogicalX + m_LogicalWidth * 0.5f; }
    float GetCenterY() const { return m_LogicalY + m_LogicalHeight * 0.5f; }
    int GetLane() const { return m_Lane; }
    bool IsPlayer() const { return m_Player; }
    SDL_Color GetLaneColor() const;

private:
    void ApplyLanePalette(const std::string& animName);
    void PlayAnim(const std::string& name, bool forceRestart = false);
    void UpdateVisualPlacement();

    AnimatedSprite m_Sprite;
    SDL_Texture* m_BaseTexture = nullptr;
    SDL_Texture* m_PaletteTexture = nullptr;
    TextureHandle m_BaseTextureGL;
    TextureHandle m_PaletteTextureGL;
    int m_Lane = 0;
    bool m_Player = false;
    bool m_Pressed = false;
    float m_ResetTimer = 0.0f;
    float m_LogicalX = 0.0f;
    float m_LogicalY = 0.0f;
    float m_LogicalWidth = 0.0f;
    float m_LogicalHeight = 0.0f;
};

} // namespace FNF
