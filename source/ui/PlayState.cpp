/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * PlayState implementation.
 */

#include "PlayState.h"
#include "FreeplayState.h"
#include "../audio/Conductor.h"
#include "../audio/MusicPlayer.h"
#include "../audio/SoundPlayer.h"
#include "../audio/VocalsPlayer.h"
#include "../core/Logger.h"
#include "../core/StateManager.h"
#include "../data/Paths.h"

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

void PlayState::LoadAssets(SDL_Renderer* renderer) {
    m_Renderer = renderer;

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

    m_Stage.Load(renderer, m_StageData);
    m_Dad.Load(renderer, m_Chart.player2);
    m_Boyfriend.Load(renderer, m_Chart.player1);
    if (!m_StageData.hideGirlfriend) {
        m_Girlfriend.Load(renderer, m_Chart.gfVersion);
    }

    m_Dad.SetPosition(m_StageData.opponentX, m_StageData.opponentY);
    m_Boyfriend.SetPosition(m_StageData.boyfriendX, m_StageData.boyfriendY);
    if (!m_StageData.hideGirlfriend) {
        m_Girlfriend.SetPosition(m_StageData.girlfriendX, m_StageData.girlfriendY);
    }

    BuildGameplayScene(renderer);

    m_AssetsLoaded = true;
}

void PlayState::BuildGameplayScene(SDL_Renderer* renderer) {
    m_Notes.clear();
    m_NoteSplashes.clear();
    m_HoldSplashes.clear();
    m_NextNoteIndex = 0;
    m_Health = 1.0f;
    m_SongHits = 0;
    m_SongMisses = 0;
    m_EndingSong = false;

    for (int lane = 0; lane < kLaneCount; ++lane) {
        m_OpponentStrums[lane].Load(renderer, lane, false);

        m_PlayerStrums[lane].Load(renderer, lane, true);
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
    m_CameraFocus = m_Dad.GetCameraFocusPoint();
    m_CameraTarget = m_CameraFocus;
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
        if (note.Load(m_Renderer, chartNote.lane, chartNote.mustHit, chartNote.strumTime, chartNote.sustainLength, m_UpScroll)) {
            note.SetStrumAnchor(chartNote.mustHit ? m_PlayerStrums[chartNote.lane] : m_OpponentStrums[chartNote.lane]);
            note.Refresh(songPositionMs, m_NoteScrollSpeed);
            m_Notes.push_back(std::move(note));
        }

        ++m_NextNoteIndex;
    }
}

void PlayState::UpdateCamera(float dt, bool focusPlayer, bool focusOpponent) {
    if (focusPlayer) {
        m_CameraTarget = m_Boyfriend.GetCameraFocusPoint();
        m_CameraTarget.x += m_StageData.cameraBoyfriendX;
        m_CameraTarget.y += m_StageData.cameraBoyfriendY;
    } else if (focusOpponent) {
        m_CameraTarget = m_Dad.GetCameraFocusPoint();
        m_CameraTarget.x += m_StageData.cameraOpponentX;
        m_CameraTarget.y += m_StageData.cameraOpponentY;
    }

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
    bool focusedPlayer = false;
    bool focusedOpponent = false;

    SpawnPendingNotes(songPosition);

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
            focusedOpponent = true;
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

    UpdateCamera(dt, focusedPlayer, focusedOpponent);

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
    m_CameraTarget = m_Boyfriend.GetCameraFocusPoint();
    m_CameraTarget.x += m_StageData.cameraBoyfriendX;
    m_CameraTarget.y += m_StageData.cameraBoyfriendY;

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
    if (!m_Renderer) {
        return;
    }

    NoteSplash splash;
    if (splash.Spawn(m_Renderer, strum, lane)) {
        m_NoteSplashes.push_back(std::move(splash));
    }
}

void PlayState::SpawnHoldSplash(const StrumNote& strum, int lane, float remainingDurationMs) {
    if (!m_Renderer || remainingDurationMs <= 0.0f) {
        return;
    }

    HoldSplash splash;
    if (splash.Spawn(m_Renderer, strum, lane, remainingDurationMs)) {
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

void PlayState::DrawText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
                         int x, int y, SDL_Color color, bool centered) const {
    if (!font || text.empty()) {
        return;
    }

    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) {
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_Rect dst = { x, y, surface->w, surface->h };
        if (centered) {
            dst.x -= surface->w / 2;
        }
        SDL_RenderCopy(renderer, texture, nullptr, &dst);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}

void PlayState::Render(SDL_Renderer* renderer) {
    if (!m_AssetsLoaded) {
        LoadAssets(renderer);
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    m_Stage.Draw(renderer, m_CamGame.scrollX, m_CamGame.scrollY, m_CamGame.zoom);
    if (!m_StageData.hideGirlfriend) {
        m_Girlfriend.Draw(renderer, m_CamGame.scrollX, m_CamGame.scrollY, m_CamGame.zoom);
    }
    m_Dad.Draw(renderer, m_CamGame.scrollX, m_CamGame.scrollY, m_CamGame.zoom);
    m_Boyfriend.Draw(renderer, m_CamGame.scrollX, m_CamGame.scrollY, m_CamGame.zoom);

    for (const Note& note : m_Notes) {
        note.Draw(renderer);
    }
    for (const StrumNote& strum : m_OpponentStrums) {
        strum.Draw(renderer);
    }
    for (const StrumNote& strum : m_PlayerStrums) {
        strum.Draw(renderer);
    }
    for (const HoldSplash& splash : m_HoldSplashes) {
        splash.Draw(renderer);
    }
    for (const NoteSplash& splash : m_NoteSplashes) {
        splash.Draw(renderer);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 155);
    SDL_Rect topBar = { 0, 0, 1280, 88 };
    SDL_RenderFillRect(renderer, &topBar);
    SDL_Rect bottomBar = { 0, 640, 1280, 80 };
    SDL_RenderFillRect(renderer, &bottomBar);

    const int healthBarX = 320;
    const int healthBarY = 18;
    const int healthBarW = 640;
    const int healthBarH = 20;
    SDL_SetRenderDrawColor(renderer, 35, 35, 35, 255);
    SDL_Rect healthBack = { healthBarX, healthBarY, healthBarW, healthBarH };
    SDL_RenderFillRect(renderer, &healthBack);
    SDL_SetRenderDrawColor(renderer, 255, 70, 80, 255);
    SDL_Rect healthLeft = { healthBarX, healthBarY, static_cast<int>(healthBarW * (1.0f - (m_Health * 0.5f))), healthBarH };
    SDL_RenderFillRect(renderer, &healthLeft);
    SDL_SetRenderDrawColor(renderer, 65, 220, 150, 255);
    SDL_Rect healthRight = { healthBarX + healthLeft.w, healthBarY, healthBarW - healthLeft.w, healthBarH };
    SDL_RenderFillRect(renderer, &healthRight);

    SDL_SetRenderDrawColor(renderer, 35, 35, 35, 255);
    SDL_Rect timeBack = { 458, 48, 364, 12 };
    SDL_RenderFillRect(renderer, &timeBack);
    const float songProgress = (m_SongEndTimeMs > 0.0f) ? std::min(1.0f, Conductor::songPosition / m_SongEndTimeMs) : 0.0f;
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    SDL_Rect timeFill = { 458, 48, static_cast<int>(364.0f * songProgress), 12 };
    SDL_RenderFillRect(renderer, &timeFill);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

}

} // namespace FNF