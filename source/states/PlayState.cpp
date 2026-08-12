/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * PlayState implementation.
 */

#include "PlayState.h"
#include "FreeplayState.h"
#include "../backend/Conductor.h"
#include "../backend/MusicPlayer.h"
#include "../backend/SoundPlayer.h"
#include "../backend/VocalsPlayer.h"
#include "../backend/Logger.h"
#include "../backend/StateManager.h"
#include "../backend/Paths.h"
#include "../backend/OpenGLESBackend.h"
#include "../backend/RenderText.h"

#include <algorithm>
#include <cmath>

namespace FNF {

namespace {

constexpr float kAutoConfirmDuration = 0.12f;
constexpr float kSpawnWindowMs = 3500.0f;
constexpr float kDespawnWindowMs = 750.0f;

}

PlayState::PlayState(PlayRequest request, SongChartData chart, StageData stage)
    : m_Request(std::move(request))
    , m_Chart(std::move(chart))
    , m_StageData(std::move(stage)) {}

void PlayState::Enter() {
    Logger::Info("[PlayState] Enter: " + m_Chart.songName);
    m_AssetsLoaded = false;
    m_StartedMusic = false;
    curBeat = 0;
    curStep = 0;
    curDecBeat = 0.0f;
    curDecStep = 0.0f;
    Conductor::ClearBPMChanges();
    Conductor::SetBPM(m_Chart.bpm);
    Conductor::songPosition = 0.0f;
}

void PlayState::Exit() {
    CloseFonts();
    MusicPlayer::Stop();
    VocalsPlayer::Stop();

    const std::string menuMusic = Paths::Music("freakyMenu");
    if (!menuMusic.empty()) {
        Conductor::ClearBPMChanges();
        Conductor::SetBPM(102.0f);
        MusicPlayer::Play(menuMusic, -1, 0.0f);
        MusicPlayer::FadeIn(600, 0.7f);
    }

    Logger::Info("[PlayState] Exit");
}

void PlayState::LoadAssets(OpenGLESBackend& renderer) {
    if (!TTF_WasInit() && TTF_Init() < 0) {
        Logger::Error("[PlayState] TTF_Init failed: " + std::string(TTF_GetError()));
    } else {
        if (!m_Font) {
            const std::string fontPath = Paths::Font("inter.otf");
            if (!fontPath.empty()) {
                m_Font = TTF_OpenFont(fontPath.c_str(), 20);
            }
        }
        if (!m_TitleFont) {
            const std::string fontPath = Paths::Font("inter-bold.otf");
            if (!fontPath.empty()) {
                m_TitleFont = TTF_OpenFont(fontPath.c_str(), 30);
            }
        }
    }

    m_Stage.LoadGL(m_StageData);
    m_Dad.LoadGL(m_Chart.player2);
    m_Boyfriend.LoadGL(m_Chart.player1);
    if (!m_StageData.hideGirlfriend) {
        m_Girlfriend.LoadGL(m_Chart.gfVersion);
    }

    m_Dad.SetPosition(m_StageData.opponentX, m_StageData.opponentY);
    m_Boyfriend.SetPosition(m_StageData.boyfriendX, m_StageData.boyfriendY);
    if (!m_StageData.hideGirlfriend) {
        m_Girlfriend.SetPosition(m_StageData.girlfriendX, m_StageData.girlfriendY);
    }

    BuildGameplayScene(renderer);

    m_AssetsLoaded = true;
}

void PlayState::BuildGameplayScene(OpenGLESBackend& renderer) {
    m_Notes.clear();
    m_NoteSplashes.clear();
    m_HoldSplashes.clear();
    m_NextNoteIndex = 0;
    m_Health = 1.0f;
    m_SongHits = 0;
    m_SongMisses = 0;
    m_EndingSong = false;

    for (int lane = 0; lane < kLaneCount; ++lane) {
        m_OpponentStrums[lane].LoadGL(lane, false);

        m_PlayerStrums[lane].LoadGL(lane, true);
    }

    ConfigurePlayfieldMetrics();
    ConfigureCameras();

    for (int lane = 0; lane < kLaneCount; ++lane) {
        m_OpponentStrums[lane].SetPosition(m_OpponentStrumBaseX + m_StrumSpacing * static_cast<float>(lane), m_OpponentStrumY);
        m_PlayerStrums[lane].SetPosition(m_PlayerStrumBaseX + m_StrumSpacing * static_cast<float>(lane), m_PlayerStrumY);
    }

    m_NoteSplashes.reserve(16);
    m_HoldSplashes.reserve(8);

    m_SongEndTimeMs = 0.0f;
    for (const ChartNote& chartNote : m_Chart.notes) {
        m_SongEndTimeMs = std::max(m_SongEndTimeMs, chartNote.strumTime + chartNote.sustainLength + 1800.0f);
    }
    m_CurrentCameraSection = -1;
    MoveCameraSection(0);
    m_CameraFocus = m_CameraTarget;
    UpdateCamera(1.0f);
}

void PlayState::ConfigurePlayfieldMetrics() {
    constexpr float kStrumLineX = 42.0f;
    constexpr float kStrumStartOffset = 50.0f;
    constexpr float kHaxeSwagWidth = 160.0f * 0.7f;
    constexpr float kStrumLineY = 50.0f;
    constexpr float kScreenWidth = 1280.0f;

    m_StrumSpacing = kHaxeSwagWidth;
    m_OpponentStrumBaseX = kStrumLineX + kStrumStartOffset;
    m_PlayerStrumBaseX = kStrumLineX + kStrumStartOffset + (kScreenWidth * 0.5f);
    m_OpponentStrumY = kStrumLineY;
    m_PlayerStrumY = kStrumLineY;

    const float pixelsPerStep = m_PlayerStrums[0].GetWidth() * 0.64f;
    m_NoteScrollSpeed = pixelsPerStep / std::max(1.0f, Conductor::stepCrochet);
}

void PlayState::ConfigureCameras() {
    m_CamGame.zoom = m_StageData.defaultZoom;
    m_CamHUD.zoom = 1.0f;
    m_CamOther.zoom = 1.0f;
}

void PlayState::SpawnPendingNotes(float songPositionMs) {
    while (m_NextNoteIndex < m_Chart.notes.size()) {
        const ChartNote& chartNote = m_Chart.notes[m_NextNoteIndex];
        if (chartNote.strumTime - songPositionMs > kSpawnWindowMs) {
            break;
        }

        Note note;
        if (note.LoadGL(chartNote.lane, chartNote.mustHit, chartNote.strumTime, chartNote.sustainLength, m_UpScroll)) {
            note.SetStrumAnchor(chartNote.mustHit ? m_PlayerStrums[chartNote.lane] : m_OpponentStrums[chartNote.lane]);
            note.Refresh(songPositionMs, m_NoteScrollSpeed);
            m_Notes.push_back(std::move(note));
        }

        ++m_NextNoteIndex;
    }
}

void PlayState::UpdateCameraSection(float songPositionMs) {
    if (m_Chart.sections.empty()) {
        return;
    }

    int sectionIndex = 0;
    for (std::size_t i = 0; i < m_Chart.sections.size(); ++i) {
        if (songPositionMs + 0.001f >= m_Chart.sections[i].startTime) {
            sectionIndex = static_cast<int>(i);
        } else {
            break;
        }
    }

    if (sectionIndex != m_CurrentCameraSection) {
        MoveCameraSection(sectionIndex);
    }
}

void PlayState::MoveCameraSection(int sectionIndex) {
    if (m_Chart.sections.empty()) {
        MoveCameraToOpponent();
        m_CurrentCameraSection = -1;
        return;
    }

    const int clampedIndex = std::max(0, std::min(sectionIndex, static_cast<int>(m_Chart.sections.size()) - 1));
    const ChartSection& section = m_Chart.sections[clampedIndex];
    m_CurrentCameraSection = clampedIndex;

    if (section.gfSection && !m_StageData.hideGirlfriend) {
        MoveCameraToGirlfriend();
    } else if (section.mustHitSection) {
        MoveCameraToPlayer();
    } else {
        MoveCameraToOpponent();
    }
}

void PlayState::MoveCameraToOpponent() {
    m_CameraTarget = m_Dad.GetCameraFocusPoint();
    m_CameraTarget.x += 150.0f + m_StageData.cameraOpponentX;
    m_CameraTarget.y += -100.0f + m_StageData.cameraOpponentY;
}

void PlayState::MoveCameraToPlayer() {
    m_CameraTarget = m_Boyfriend.GetCameraFocusPoint();
    m_CameraTarget.x += -100.0f + m_StageData.cameraBoyfriendX;
    m_CameraTarget.y += -100.0f + m_StageData.cameraBoyfriendY;
}

void PlayState::MoveCameraToGirlfriend() {
    m_CameraTarget = m_Girlfriend.GetCameraFocusPoint();
    m_CameraTarget.x += m_StageData.cameraGirlfriendX;
    m_CameraTarget.y += m_StageData.cameraGirlfriendY;
}

void PlayState::UpdateCamera(float dt) {
    const float lerp = std::min(1.0f, dt * 3.5f * std::max(0.2f, m_StageData.cameraSpeed));
    m_CameraFocus.x += (m_CameraTarget.x - m_CameraFocus.x) * lerp;
    m_CameraFocus.y += (m_CameraTarget.y - m_CameraFocus.y) * lerp;

    m_CamGame.scrollX = m_CameraFocus.x - (1280.0f / (2.0f * m_CamGame.zoom));
    m_CamGame.scrollY = m_CameraFocus.y - (720.0f / (2.0f * m_CamGame.zoom));
}

void PlayState::CloseFonts() {
    if (m_TitleFont) {
        TTF_CloseFont(m_TitleFont);
        m_TitleFont = nullptr;
    }
    if (m_Font) {
        TTF_CloseFont(m_Font);
        m_Font = nullptr;
    }
}

void PlayState::HandleEvent(const SDL_Event& e) {
    if (e.type == SDL_KEYDOWN && !e.key.repeat) {
        switch (e.key.keysym.sym) {
            case SDLK_ESCAPE:
                {
                    const std::string sfx = Paths::Sound("cancelMenu");
                    if (!sfx.empty()) {
                        SoundPlayer::Play(sfx, 1.0f);
                    }
                    StateManager::Get().SwitchWithFade(std::make_unique<FreeplayState>(), 0.7f);
                }
                break;
            case SDLK_LEFT:
                HandleLanePress(0);
                break;
            case SDLK_DOWN:
                HandleLanePress(1);
                break;
            case SDLK_UP:
                HandleLanePress(2);
                break;
            case SDLK_RIGHT:
                HandleLanePress(3);
                break;
            default:
                break;
        }
    } else if (e.type == SDL_KEYUP) {
        switch (e.key.keysym.sym) {
            case SDLK_LEFT:
                HandleLaneRelease(0);
                break;
            case SDLK_DOWN:
                HandleLaneRelease(1);
                break;
            case SDLK_UP:
                HandleLaneRelease(2);
                break;
            case SDLK_RIGHT:
                HandleLaneRelease(3);
                break;
            default:
                break;
        }
    }
}

void PlayState::Update(float dt) {
    if (!m_AssetsLoaded) {
        return;
    }

    if (!m_StartedMusic) {
        m_StartedMusic = true;
        MusicPlayer::Play(m_Request.instPath, 0, 0.8f);
        if (m_Chart.needsVoices) {
            VocalsPlayer::Play(m_Request.playerVoicesPath, m_Request.opponentVoicesPath, 0.85f);
        }
    }

    MusicBeatState::Update(dt);

    const float songPosition = Conductor::songPosition;

    SpawnPendingNotes(songPosition);
    UpdateCameraSection(songPosition);

    for (StrumNote& strum : m_OpponentStrums) {
        strum.Update(dt);
    }
    for (StrumNote& strum : m_PlayerStrums) {
        strum.Update(dt);
    }

    for (Note& note : m_Notes) {
        if (!note.IsAlive()) {
            continue;
        }

        note.Refresh(songPosition, m_NoteScrollSpeed);

        if (note.MustHit()) {
            if (note.IsLate(songPosition)) {
                note.MarkMissed();
                m_Health = std::max(0.0f, m_Health - 0.07f);
                ++m_SongMisses;
            }
            continue;
        }

        if (songPosition >= note.GetStrumTime()) {
            m_Dad.Sing(note.GetLane(), note.HasSustain() ? note.GetSustainLength() / 1000.0f : 0.0f);
            m_OpponentStrums[note.GetLane()].Confirm(kAutoConfirmDuration);
            if (note.HasSustain()) {
                SpawnHoldSplash(m_OpponentStrums[note.GetLane()], note.GetLane(), note.GetSustainLength());
            }
            note.MarkHit();
            m_Health = std::max(0.0f, m_Health - 0.015f);
        }
    }

    for (NoteSplash& splash : m_NoteSplashes) {
        splash.Update(dt);
    }
    for (HoldSplash& splash : m_HoldSplashes) {
        splash.Update(dt);
    }

    m_Notes.erase(std::remove_if(m_Notes.begin(), m_Notes.end(), [](const Note& note) {
        return !note.IsAlive();
    }), m_Notes.end());

    m_NoteSplashes.erase(std::remove_if(m_NoteSplashes.begin(), m_NoteSplashes.end(), [](const NoteSplash& splash) {
        return !splash.IsAlive();
    }), m_NoteSplashes.end());

    m_HoldSplashes.erase(std::remove_if(m_HoldSplashes.begin(), m_HoldSplashes.end(), [](const HoldSplash& splash) {
        return !splash.IsAlive();
    }), m_HoldSplashes.end());

    m_Dad.Update(dt);
    m_Boyfriend.Update(dt);
    if (!m_StageData.hideGirlfriend) {
        m_Girlfriend.Update(dt);
    }

    UpdateCamera(dt);

    m_Notes.erase(std::remove_if(m_Notes.begin(), m_Notes.end(), [songPosition](const Note& note) {
        return !note.IsAlive() || songPosition > (note.GetStrumTime() + kDespawnWindowMs);
    }), m_Notes.end());

    if (!m_EndingSong && songPosition >= m_SongEndTimeMs) {
        EndSong();
    }
}

void PlayState::BeatHit() {
    m_Dad.Dance();
    m_Boyfriend.Dance();
    if (!m_StageData.hideGirlfriend) {
        m_Girlfriend.Dance();
    }
}

void PlayState::HandleLanePress(int lane) {
    if (!m_AssetsLoaded || lane < 0 || lane >= kLaneCount) {
        return;
    }

    m_PlayerStrums[lane].Press();

    Note* bestNote = nullptr;
    float bestDistance = Note::kDefaultHitWindowMs + 1.0f;
    for (Note& note : m_Notes) {
        if (!note.IsAlive() || !note.MustHit() || note.GetLane() != lane) {
            continue;
        }

        if (!note.CanBeHit(Conductor::songPosition)) {
            continue;
        }

        const float distance = std::abs(Conductor::songPosition - note.GetStrumTime());
        if (distance < bestDistance) {
            bestDistance = distance;
            bestNote = &note;
        }
    }

    if (!bestNote) {
        return;
    }

    bestNote->MarkHit();
    m_PlayerStrums[lane].Confirm(kAutoConfirmDuration);
    SpawnNoteSplash(m_PlayerStrums[lane], lane);
    ++m_SongHits;
    m_Health = std::min(2.0f, m_Health + 0.035f);

    if (bestNote->HasSustain()) {
        const float remainingDuration = std::max(0.0f, bestNote->GetHoldEndTime() - Conductor::songPosition);
        m_Boyfriend.Hold(lane, remainingDuration / 1000.0f);
        SpawnHoldSplash(m_PlayerStrums[lane], lane, remainingDuration);
    } else {
        m_Boyfriend.Sing(lane);
    }
}

void PlayState::HandleLaneRelease(int lane) {
    if (!m_AssetsLoaded || lane < 0 || lane >= kLaneCount) {
        return;
    }

    m_PlayerStrums[lane].Release();
    m_Boyfriend.StopHold(lane);
}

void PlayState::SpawnNoteSplash(const StrumNote& strum, int lane) {
    NoteSplash splash;
    if (splash.SpawnGL(strum, lane)) {
        m_NoteSplashes.push_back(std::move(splash));
    }
}

void PlayState::SpawnHoldSplash(const StrumNote& strum, int lane, float remainingDurationMs) {
    if (remainingDurationMs <= 0.0f) {
        return;
    }

    HoldSplash splash;
    if (splash.SpawnGL(strum, lane, remainingDurationMs)) {
        m_HoldSplashes.push_back(std::move(splash));
    }
}

void PlayState::EndSong() {
    if (m_EndingSong) {
        return;
    }

    m_EndingSong = true;
    MusicPlayer::Stop();
    VocalsPlayer::Stop();
    StateManager::Get().SwitchWithFade(std::make_unique<FreeplayState>(), 0.45f);
}

std::string PlayState::FormatSongTime(float timeMs) const {
    const int totalSeconds = std::max(0, static_cast<int>(timeMs / 1000.0f));
    const int minutes = totalSeconds / 60;
    const int seconds = totalSeconds % 60;

    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%d:%02d", minutes, seconds);
    return std::string(buffer);
}

void PlayState::DrawText(OpenGLESBackend& renderer, TTF_Font* font, const std::string& text,
                         int x, int y, SDL_Color color, bool centered) const {
    RenderText::Draw(renderer, font, text, x, y, color, centered);
}

void PlayState::Render(OpenGLESBackend& renderer) {
    if (!m_AssetsLoaded) {
        LoadAssets(renderer);
    }

    m_Stage.DrawGL(renderer, m_CamGame.scrollX, m_CamGame.scrollY, m_CamGame.zoom);
    if (!m_StageData.hideGirlfriend) {
        m_Girlfriend.DrawGL(renderer, m_CamGame.scrollX, m_CamGame.scrollY, m_CamGame.zoom);
    }
    m_Dad.DrawGL(renderer, m_CamGame.scrollX, m_CamGame.scrollY, m_CamGame.zoom);
    m_Boyfriend.DrawGL(renderer, m_CamGame.scrollX, m_CamGame.scrollY, m_CamGame.zoom);

    for (const Note& note : m_Notes) {
        note.DrawGL(renderer);
    }
    for (const StrumNote& strum : m_OpponentStrums) {
        strum.DrawGL(renderer);
    }
    for (const StrumNote& strum : m_PlayerStrums) {
        strum.DrawGL(renderer);
    }
    for (const HoldSplash& splash : m_HoldSplashes) {
        splash.DrawGL(renderer);
    }
    for (const NoteSplash& splash : m_NoteSplashes) {
        splash.DrawGL(renderer);
    }

    renderer.FillRect({0.0f, 0.0f, 1280.0f, 88.0f}, {0.0f, 0.0f, 0.0f, 155.0f / 255.0f});
    renderer.FillRect({0.0f, 640.0f, 1280.0f, 80.0f}, {0.0f, 0.0f, 0.0f, 155.0f / 255.0f});

    const int healthBarX = 320;
    const int healthBarY = 18;
    const int healthBarW = 640;
    const int healthBarH = 20;
    renderer.FillRect({static_cast<float>(healthBarX), static_cast<float>(healthBarY), static_cast<float>(healthBarW), static_cast<float>(healthBarH)}, {35.0f / 255.0f, 35.0f / 255.0f, 35.0f / 255.0f, 1.0f});
    const int healthLeftW = static_cast<int>(healthBarW * (1.0f - (m_Health * 0.5f)));
    renderer.FillRect({static_cast<float>(healthBarX), static_cast<float>(healthBarY), static_cast<float>(healthLeftW), static_cast<float>(healthBarH)}, {1.0f, 70.0f / 255.0f, 80.0f / 255.0f, 1.0f});
    renderer.FillRect({static_cast<float>(healthBarX + healthLeftW), static_cast<float>(healthBarY), static_cast<float>(healthBarW - healthLeftW), static_cast<float>(healthBarH)}, {65.0f / 255.0f, 220.0f / 255.0f, 150.0f / 255.0f, 1.0f});

    renderer.FillRect({458.0f, 48.0f, 364.0f, 12.0f}, {35.0f / 255.0f, 35.0f / 255.0f, 35.0f / 255.0f, 1.0f});
    const float songProgress = (m_SongEndTimeMs > 0.0f) ? std::min(1.0f, Conductor::songPosition / m_SongEndTimeMs) : 0.0f;
    renderer.FillRect({458.0f, 48.0f, 364.0f * songProgress, 12.0f}, {220.0f / 255.0f, 220.0f / 255.0f, 220.0f / 255.0f, 1.0f});

}

} // namespace FNF
