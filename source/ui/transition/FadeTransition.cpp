/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * FadeTransition Implementation
 *
 * Author: LeninAsto
 * Date: March 2026
 */

#include "FadeTransition.h"
#include "../../audio/SoundPlayer.h"
#include "../../data/Paths.h"
#include <SDL2/SDL.h>
#include <cmath>

namespace FNF {

FadeTransition::FadeTransition(float duration, bool isTransIn, Callback onFinish)
    : m_Duration(duration)
    , m_IsTransIn(isTransIn)
    , m_OnFinish(std::move(onFinish))
{}

void FadeTransition::Enter() {
    m_GradientH = static_cast<float>(SCR_H);
    m_Done      = false;

    if (!m_IsTransIn) {
        const std::string sfx = Paths::Sound("FadeTransition");
        if (!sfx.empty()) {
            SoundPlayer::Play(sfx, 0.4f);
        }
    }

    // Both fade-in and fade-out start with the gradient strip above the screen.
    // For fade-out: the solid black bar is ABOVE the gradient (also above screen)
    //   → screen starts fully clear, black sweeps in from top.
    // For fade-in:  the solid black bar is BELOW the gradient.
    //   gradient.y = -(SCR_H), bar.y = gradient.y + gradient.h = 0
    //   → bar covers y=0..BAR_H at first frame → screen starts fully black.
    m_GradientY = -m_GradientH;
}

void FadeTransition::Update(float dt) {
    if (m_Done) return;

    // Advance gradient position (same formula as Haxe)
    const float targetPos = m_GradientH + 50.0f;
    if (m_Duration > 0.0f)
        m_GradientY += (static_cast<float>(SCR_H) + targetPos) * dt / m_Duration;
    else
        m_GradientY = targetPos;

    if (m_GradientY >= targetPos) {
        m_Done = true;
        Close();
        if (m_OnFinish) m_OnFinish();
    }
}

void FadeTransition::Render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    const int gradY = static_cast<int>(m_GradientY);
    const int gradH = static_cast<int>(m_GradientH);

    if (m_IsTransIn) {
        // Fade-IN: solid black bar BELOW the gradient strip
        SDL_Rect bar = { 0, gradY + gradH, SCR_W, BAR_H };
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &bar);

        // Gradient: TRANSPARENT(top) → BLACK(bottom)
        for (int y = 0; y < gradH; ++y) {
            float t   = static_cast<float>(y) / static_cast<float>(gradH);
            auto  a   = static_cast<Uint8>(255.0f * t);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, a);
            SDL_Rect line = { 0, gradY + y, SCR_W, 1 };
            SDL_RenderFillRect(renderer, &line);
        }
    } else {
        // Fade-OUT: solid black bar ABOVE the gradient strip
        SDL_Rect bar = { 0, gradY - BAR_H, SCR_W, BAR_H };
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &bar);

        // Gradient: BLACK(top) → TRANSPARENT(bottom)
        for (int y = 0; y < gradH; ++y) {
            float t   = static_cast<float>(y) / static_cast<float>(gradH);
            auto  a   = static_cast<Uint8>(255.0f * (1.0f - t));
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, a);
            SDL_Rect line = { 0, gradY + y, SCR_W, 1 };
            SDL_RenderFillRect(renderer, &line);
        }
    }

    // Restore blend mode for subsequent rendering
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

} // namespace FNF
