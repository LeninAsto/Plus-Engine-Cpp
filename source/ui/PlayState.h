/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Minimal gameplay scene.
 */

#pragma once

#include "../core/MusicBeatState.h"
#include "../play/HoldSplash.h"
#include "../play/Note.h"
#include "../play/NoteSplash.h"
#include "../play/Character.h"
#include "../play/PlayRequest.h"
#include "../play/SongChart.h"
#include "../play/Stage.h"
#include <SDL2/SDL_ttf.h>
#include <array>
#include <vector>

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
    struct CameraLayer {
        float scrollX = 0.0f;
        float scrollY = 0.0f;
        float zoom = 1.0f;
    };

    static constexpr int kLaneCount = 4;

    PlayRequest m_Request;
    SongChartData m_Chart;
    StageData m_StageData;

    bool m_AssetsLoaded = false;
    bool m_StartedMusic = false;
    SDL_Renderer* m_Renderer = nullptr;

    StageScene m_Stage;
    Character m_Dad;
    Character m_Boyfriend;
    Character m_Girlfriend;

    std::array<StrumNote, kLaneCount> m_PlayerStrums;
    std::array<StrumNote, kLaneCount> m_OpponentStrums;
    std::vector<Note> m_Notes;
    std::vector<NoteSplash> m_NoteSplashes;
    std::vector<HoldSplash> m_HoldSplashes;
    std::size_t m_NextNoteIndex = 0;

    CameraLayer m_CamGame;
    CameraLayer m_CamHUD;
    CameraLayer m_CamOther;
    SDL_FPoint m_CameraTarget = { 0.0f, 0.0f };
    SDL_FPoint m_CameraFocus = { 0.0f, 0.0f };

    float m_NoteScrollSpeed = 0.45f;
    float m_StrumSpacing = 112.0f;
    float m_PlayerStrumBaseX = 712.0f;
    float m_OpponentStrumBaseX = 120.0f;
    float m_PlayerStrumY = 96.0f;
    float m_OpponentStrumY = 96.0f;
    bool m_UpScroll = true;
    float m_Health = 1.0f;
    int m_SongHits = 0;
    int m_SongMisses = 0;
    float m_SongEndTimeMs = 0.0f;
    bool m_EndingSong = false;

    TTF_Font* m_Font = nullptr;
    TTF_Font* m_TitleFont = nullptr;

    void LoadAssets(SDL_Renderer* renderer);
    void BuildGameplayScene(SDL_Renderer* renderer);
    void ConfigurePlayfieldMetrics();
    void ConfigureCameras();
    void CloseFonts();
    void SpawnPendingNotes(float songPositionMs);
    void UpdateCamera(float dt, bool focusPlayer, bool focusOpponent);
    void HandleLanePress(int lane);
    void HandleLaneRelease(int lane);
    void SpawnNoteSplash(const StrumNote& strum, int lane);
    void SpawnHoldSplash(const StrumNote& strum, int lane, float remainingDurationMs);
    void EndSong();
    std::string FormatSongTime(float timeMs) const;
    void DrawText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
                  int x, int y, SDL_Color color, bool centered = false) const;
};

} // namespace FNF