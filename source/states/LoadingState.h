/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Minimal LoadingState that precaches images and music before PlayState.
 */

#pragma once

#include "../backend/State.h"
#include "../objects/Sprite.h"
#include "../objects/PlayRequest.h"
#include "../objects/SongChart.h"
#include "../objects/Stage.h"
#include <SDL2/SDL_ttf.h>
#include <optional>
#include <string>

namespace FNF {

class OpenGLESBackend;

class LoadingState : public State {
public:
    explicit LoadingState(PlayRequest request);

    void Enter() override;
    void Exit() override;
    void Update(float dt) override;
    void Render(OpenGLESBackend& renderer) override;

private:
    enum class Step {
        LoadChart,
        LoadStage,
        PrecacheStage,
        PrecacheCharacters,
        PrecacheMusic,
        Done,
        Failed
    };

    PlayRequest m_Request;
    std::optional<SongChartData> m_Chart;
    std::optional<StageData> m_Stage;
    Step m_Step = Step::LoadChart;
    float m_Progress = 0.0f;
    std::string m_Status = "Preparing...";

    bool m_AssetsLoaded = false;
    Sprite m_Background;
    TTF_Font* m_Font = nullptr;
    TTF_Font* m_TitleFont = nullptr;

    void LoadAssets(OpenGLESBackend& renderer);
    void CloseFonts();
    void Advance(OpenGLESBackend& renderer);
    void DrawText(OpenGLESBackend& renderer, TTF_Font* font, const std::string& text,
                  int x, int y, SDL_Color color, bool centered = false) const;
};

} // namespace FNF
