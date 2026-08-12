/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Minimal animated character wrapper.
 */

#pragma once

#include "../objects/AnimatedSprite.h"
#include <SDL2/SDL.h>
#include <string>
#include <vector>

namespace FNF {

class OpenGLESBackend;

class Character {
public:
    bool Load(SDL_Renderer* renderer, const std::string& characterId);
    bool Precache(SDL_Renderer* renderer, const std::string& characterId);
    bool LoadGL(const std::string& characterId);
    bool PrecacheGL(const std::string& characterId);

    void Update(float dt);
    void Draw(SDL_Renderer* renderer) const;
    void Draw(SDL_Renderer* renderer, float cameraX, float cameraY, float zoom) const;
    void DrawGL(OpenGLESBackend& backend) const;
    void DrawGL(OpenGLESBackend& backend, float cameraX, float cameraY, float zoom) const;
    void Dance();
    void Sing(int lane, float holdDuration = 0.0f);
    void Hold(int lane, float duration);
    void StopHold(int lane = -1);
    bool IsHolding() const { return m_HoldTimer > 0.0f; }
    void SetPosition(float px, float py);
    SDL_FPoint GetCameraFocusPoint() const;
    float GetSingDurationSeconds() const { return m_SingDurationSeconds; }
    const std::string& GetVocalsFile() const { return m_VocalsFile; }

    float x = 0.0f;
    float y = 0.0f;

private:
    struct AnimationDef {
        std::string anim;
        std::string name;
        std::vector<int> indices;
        int fps = 24;
        bool loop = false;
        float offX = 0.0f;
        float offY = 0.0f;
    };

    AnimatedSprite m_Sprite;
    std::vector<AnimationDef> m_Animations;
    float m_PosOffsetX = 0.0f;
    float m_PosOffsetY = 0.0f;
    float m_CurrentAnimOffsetX = 0.0f;
    float m_CurrentAnimOffsetY = 0.0f;
    float m_Scale = 1.0f;
    bool m_DanceToggle = false;
    float m_HoldTimer = 0.0f;
    int m_HoldLane = -1;
    float m_CameraOffsetX = 0.0f;
    float m_CameraOffsetY = 0.0f;
    float m_SingDurationSeconds = 0.6f;
    std::string m_VocalsFile;

    bool LoadInternal(SDL_Renderer* renderer, const std::string& characterId, bool keepConfiguredSprite);
    bool LoadInternalGL(const std::string& characterId, bool keepConfiguredSprite);
    bool HasAnimation(const std::string& animName) const;
    void PlayAnimation(const std::string& animName, bool forceRestart);
};

} // namespace FNF
