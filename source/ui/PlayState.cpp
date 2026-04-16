/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * PlayState implementation.
 */

#include "PlayState.h"
#include "FreeplayState.h"
#include "../audio/Conductor.h"
#include "../audio/MusicPlayer.h"
#include "../audio/SoundPlayer.h"
#include "../core/Logger.h"
#include "../core/StateManager.h"
#include "../data/Paths.h"

namespace FNF {

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

    m_AssetsLoaded = true;
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
    if (e.type != SDL_KEYDOWN) {
        return;
    }

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
            m_Boyfriend.Sing(0);
            break;
        case SDLK_DOWN:
            m_Boyfriend.Sing(1);
            break;
        case SDLK_UP:
            m_Boyfriend.Sing(2);
            break;
        case SDLK_RIGHT:
            m_Boyfriend.Sing(3);
            break;
        default:
            break;
    }
}

void PlayState::Update(float dt) {
    if (!m_AssetsLoaded) {
        return;
    }

    if (!m_StartedMusic) {
        m_StartedMusic = true;
        MusicPlayer::Play(m_Request.instPath, 0, 0.8f);
    }

    m_Dad.Update(dt);
    m_Boyfriend.Update(dt);
    if (!m_StageData.hideGirlfriend) {
        m_Girlfriend.Update(dt);
    }

    MusicBeatState::Update(dt);
}

void PlayState::BeatHit() {
    m_Dad.Dance();
    m_Boyfriend.Dance();
    if (!m_StageData.hideGirlfriend) {
        m_Girlfriend.Dance();
    }
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

    m_Stage.Draw(renderer);
    if (!m_StageData.hideGirlfriend) {
        m_Girlfriend.Draw(renderer);
    }
    m_Dad.Draw(renderer);
    m_Boyfriend.Draw(renderer);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 155);
    SDL_Rect topBar = { 0, 0, 1280, 88 };
    SDL_RenderFillRect(renderer, &topBar);
    SDL_Rect bottomBar = { 0, 640, 1280, 80 };
    SDL_RenderFillRect(renderer, &bottomBar);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 55);
    for (int i = 0; i < 4; ++i) {
        SDL_Rect receptor = { 690 + i * 92, 560, 58, 58 };
        SDL_RenderFillRect(renderer, &receptor);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    const SDL_Color white = { 255, 255, 255, 255 };
    const SDL_Color soft = { 220, 220, 220, 255 };
    DrawText(renderer, m_TitleFont, m_Chart.songName, 34, 20, white, false);
    DrawText(renderer, m_Font, "Stage: " + m_StageData.stageName + "   BPM: " + std::to_string(static_cast<int>(m_Chart.bpm)), 34, 56, soft, false);
    DrawText(renderer, m_Font, "ESC back to Freeplay   Arrow keys make Boyfriend sing", 640, 669, white, true);
}

} // namespace FNF