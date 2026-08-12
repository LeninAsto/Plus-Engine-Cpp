/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Minimal gameplay scene.
 */

#pragma once

#include "../backend/MusicBeatState.h"
#include "../backend/PsychCamera.h"
#include "../objects/HoldSplash.h"
#include "../objects/Note.h"
#include "../objects/NoteSplash.h"
#include "../objects/Character.h"
#include "../objects/PlayRequest.h"
#include "../objects/SongChart.h"
#include "../objects/Stage.h"
#include <SDL2/SDL_ttf.h>
#include <array>
#include <vector>

namespace FNF {

class OpenGLESBackend;

class PlayState : public MusicBeatState {
public:
    PlayState(PlayRequest request, SongChartData chart, StageData stage);

    void Enter() override;
    void Exit() override;
    void HandleEvent(const SDL_Event& e) override;
    void Update(float dt) override;
    void Render(OpenGLESBackend& renderer) override;

protected:
    void BeatHit() override;

private:
    static constexpr int kLaneCount = 4;

    PlayRequest m_Request;
    SongChartData m_Chart;
    StageData m_StageData;

    bool m_AssetsLoaded = false;
    bool m_StartedMusic = false;
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

    PsychCamera m_CamGame;
    PsychCamera m_CamHUD;
    PsychCamera m_CamOther;
    SDL_FPoint m_CameraTarget = { 0.0f, 0.0f };
    SDL_FPoint m_CameraFocus = { 0.0f, 0.0f };
    int m_CurrentCameraSection = -1;

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

    void LoadAssets(OpenGLESBackend& renderer);
    void BuildGameplayScene(OpenGLESBackend& renderer);
    void ConfigurePlayfieldMetrics();
    void ConfigureCameras();
    void CloseFonts();
    void SpawnPendingNotes(float songPositionMs);
    void UpdateCameraSection(float songPositionMs);
    void MoveCameraSection(int sectionIndex);
    void MoveCameraToOpponent();
    void MoveCameraToPlayer();
    void MoveCameraToGirlfriend();
    void UpdateCamera(float dt);
    void HandleLanePress(int lane);
    void HandleLaneRelease(int lane);
    void SpawnNoteSplash(const StrumNote& strum, int lane);
    void SpawnHoldSplash(const StrumNote& strum, int lane, float remainingDurationMs);
    void EndSong();
    std::string FormatSongTime(float timeMs) const;
    void DrawText(OpenGLESBackend& renderer, TTF_Font* font, const std::string& text,
                  int x, int y, SDL_Color color, bool centered = false) const;
};

} // namespace FNF
