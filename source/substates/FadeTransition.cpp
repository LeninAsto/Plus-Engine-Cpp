/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * FadeTransition Implementation
 */

#include "FadeTransition.h"
#include "../backend/SoundPlayer.h"
#include "../backend/Paths.h"
#include "../backend/OpenGLESBackend.h"

#include <cmath>

namespace FNF {

FadeTransition::FadeTransition(float duration, bool isTransIn, Callback onFinish)
    : m_Duration(duration)
    , m_IsTransIn(isTransIn)
    , m_OnFinish(std::move(onFinish))
{}

void FadeTransition::Enter() {
    m_GradientH = static_cast<float>(SCR_H);
    m_Done = false;

    if (!m_IsTransIn) {
        const std::string sfx = Paths::Sound("FadeTransition");
        if (!sfx.empty()) {
            SoundPlayer::Play(sfx, 0.4f);
        }
    }

    m_GradientY = -m_GradientH;
}

void FadeTransition::Update(float dt) {
    if (m_Done) return;

    const float targetPos = m_GradientH + 50.0f;
    if (m_Duration > 0.0f) {
        m_GradientY += (static_cast<float>(SCR_H) + targetPos) * dt / m_Duration;
    } else {
        m_GradientY = targetPos;
    }

    if (m_GradientY >= targetPos) {
        m_Done = true;
        Close();
        if (m_OnFinish) m_OnFinish();
    }
}

void FadeTransition::Render(OpenGLESBackend& renderer) {
    const int gradY = static_cast<int>(m_GradientY);
    const int gradH = static_cast<int>(m_GradientH);

    if (m_IsTransIn) {
        renderer.FillRect({0.0f, static_cast<float>(gradY + gradH), static_cast<float>(SCR_W), static_cast<float>(BAR_H)},
                          {0.0f, 0.0f, 0.0f, 1.0f});

        for (int y = 0; y < gradH; ++y) {
            const float t = static_cast<float>(y) / static_cast<float>(gradH);
            renderer.FillRect({0.0f, static_cast<float>(gradY + y), static_cast<float>(SCR_W), 1.0f},
                              {0.0f, 0.0f, 0.0f, t});
        }
    } else {
        renderer.FillRect({0.0f, static_cast<float>(gradY - BAR_H), static_cast<float>(SCR_W), static_cast<float>(BAR_H)},
                          {0.0f, 0.0f, 0.0f, 1.0f});

        for (int y = 0; y < gradH; ++y) {
            const float t = static_cast<float>(y) / static_cast<float>(gradH);
            renderer.FillRect({0.0f, static_cast<float>(gradY + y), static_cast<float>(SCR_W), 1.0f},
                              {0.0f, 0.0f, 0.0f, 1.0f - t});
        }
    }
}

} // namespace FNF
