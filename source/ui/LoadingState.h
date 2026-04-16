/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Minimal LoadingState that precaches images and music before PlayState.
 */

#pragma once

#include "../core/State.h"
#include "../graphics/Sprite.h"
#include "../play/PlayRequest.h"
#include "../play/SongChart.h"
#include "../play/Stage.h"
#include <SDL2/SDL_ttf.h>
#include <optional>
#include <string>

namespace FNF {

class LoadingState : public State {
public:
    explicit LoadingState(PlayRequest request);

    void Enter() override;
    void Exit() override;
    void Update(float dt) override;
    void Render(SDL_Renderer* renderer) override;

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

    void LoadAssets(SDL_Renderer* renderer);
    void CloseFonts();
    void Advance(SDL_Renderer* renderer);
    void DrawText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
                  int x, int y, SDL_Color color, bool centered = false) const;
};

} // namespace FNF