/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * FadeTransition - Black fade overlay sub-state
 *
 * Mirrors CustomFadeTransition.hx from the Haxe codebase.
 *
 * Two modes:
 *   isTransIn=false  (FadeOUT): a black block sweeps down from top, covering
 *                               the screen in black. Callback fires when done.
 *   isTransIn=true   (FadeIN):  the black block that fills the screen retreats
 *                               downward, revealing the new state underneath.
 *
 * Visual breakdown (both sweep downward at equal speed):
 *   FadeOUT: [BLACK BAR above] [BLACK→TRANSPARENT gradient] [clear below]
 *   FadeIN:  [clear above]     [TRANSPARENT→BLACK gradient] [BLACK BAR below]
 *
 * Author: LeninAsto
 * Date: March 2026
 */

#pragma once

#include "../backend/SubState.h"
#include <functional>

namespace FNF {

class FadeTransition : public SubState {
public:
    using Callback = std::function<void()>;

    /**
     * @param duration  Transition time in seconds.
     * @param isTransIn true = reveal new state from black (fade-in).
     *                  false = cover current state with black (fade-out).
     * @param onFinish  Called once when the animation completes.
     */
    FadeTransition(float duration, bool isTransIn, Callback onFinish = nullptr);

    void Enter()  override;
    void Update(float dt) override;
    void Render(OpenGLESBackend& renderer) override;

private:
    static constexpr int SCR_W     = 1280;
    static constexpr int SCR_H     = 720;
    static constexpr int BAR_H     = SCR_H + 400; // oversized so edges never show

    float    m_Duration;
    bool     m_IsTransIn;
    Callback m_OnFinish;

    float m_GradientY = 0.0f;  // top-left Y of the gradient strip
    float m_GradientH = 0.0f;  // height of the gradient strip (== SCR_H)
    bool  m_Done      = false;
};

} // namespace FNF
