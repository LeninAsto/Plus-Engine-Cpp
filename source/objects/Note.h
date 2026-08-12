#pragma once

#include "StrumNote.h"

#include <array>

namespace FNF {

class Note {
public:
    static constexpr float kScale = 0.7f;
    static constexpr float kDefaultHitWindowMs = 150.0f;

    bool Load(SDL_Renderer* renderer, int lane, bool mustHit, float strumTime, float sustainLength, bool upScroll);
    bool LoadGL(int lane, bool mustHit, float strumTime, float sustainLength, bool upScroll);
    void SetStrumAnchor(const StrumNote& strum);
    void Refresh(float songPositionMs, float pixelsPerMs);
    void Draw(SDL_Renderer* renderer) const;
    void DrawGL(OpenGLESBackend& backend) const;
    static void InvalidateSharedResources();

    bool CanBeHit(float songPositionMs, float hitWindowMs = kDefaultHitWindowMs) const;
    bool IsLate(float songPositionMs, float lateWindowMs = kDefaultHitWindowMs) const;

    void MarkHit();
    void MarkMissed();

    bool IsAlive() const { return m_Alive; }
    bool HasSustain() const { return m_SustainLength > 1.0f; }
    bool MustHit() const { return m_MustHit; }
    int GetLane() const { return m_Lane; }

    float GetStrumTime() const { return m_StrumTime; }
    float GetSustainLength() const { return m_SustainLength; }
    float GetHoldEndTime() const { return m_StrumTime + m_SustainLength; }
    float GetScreenY() const { return m_Head.y; }

private:
    static bool EnsureTemplates(SDL_Renderer* renderer);
    static bool EnsureTemplatesGL();

    AnimatedSprite m_Head;
    AnimatedSprite m_Body;
    AnimatedSprite m_Tail;

    int m_Lane = 0;
    bool m_MustHit = false;
    bool m_Alive = true;
    float m_StrumTime = 0.0f;
    float m_SustainLength = 0.0f;
    float m_StrumX = 0.0f;
    float m_StrumY = 0.0f;
    float m_StrumWidth = 0.0f;
    float m_SustainPixels = 0.0f;
    bool m_UpScroll = true;

    static bool s_TemplatesLoaded;
    static bool s_TemplatesLoadedGL;
    static std::array<AnimatedSprite, 4> s_HeadTemplates;
    static std::array<AnimatedSprite, 4> s_BodyTemplates;
    static std::array<AnimatedSprite, 4> s_TailTemplates;
};

} // namespace FNF
