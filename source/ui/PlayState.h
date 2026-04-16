/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Minimal gameplay scene.
 */

#pragma once

#include "../core/MusicBeatState.h"
#include "../play/Character.h"
#include "../play/PlayRequest.h"
#include "../play/SongChart.h"
#include "../play/Stage.h"
#include <SDL2/SDL_ttf.h>

namespace FNF {

class PlayState : public MusicBeatState {
public:
    PlayState(PlayRequest request, SongChartData chart, StageData stage);

    void Enter() override;
    void Exit() override;
    void HandleEvent(const SDL_Event& e) override;
    void Update(float dt) override;
    void Render(SDL_Renderer* renderer) override;

protected:
    void BeatHit() override;

private:
    PlayRequest m_Request;
    SongChartData m_Chart;
    StageData m_StageData;

    bool m_AssetsLoaded = false;
    bool m_StartedMusic = false;

    StageScene m_Stage;
    Character m_Dad;
    Character m_Boyfriend;
    Character m_Girlfriend;

    TTF_Font* m_Font = nullptr;
    TTF_Font* m_TitleFont = nullptr;

    void LoadAssets(SDL_Renderer* renderer);
    void CloseFonts();
    void DrawText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
                  int x, int y, SDL_Color color, bool centered = false) const;
};

} // namespace FNF