/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * LoadingState implementation.
 */

#include "LoadingState.h"
#include "FreeplayState.h"
#include "PlayState.h"
#include "../backend/MusicPlayer.h"
#include "../backend/VocalsPlayer.h"
#include "../backend/Logger.h"
#include "../backend/StateManager.h"
#include "../backend/Paths.h"
#include "../objects/Character.h"
#include "../backend/OpenGLESBackend.h"
#include "../backend/RenderText.h"

namespace FNF {

LoadingState::LoadingState(PlayRequest request)
    : m_Request(std::move(request)) {}

void LoadingState::Enter() {
    Logger::Info("[LoadingState] Enter for: " + m_Request.songName);
    m_Step = Step::LoadChart;
    m_Progress = 0.0f;
    m_Status = "Loading chart...";
}

void LoadingState::Exit() {
    CloseFonts();
    Logger::Info("[LoadingState] Exit");
}

void LoadingState::LoadAssets(OpenGLESBackend& renderer) {
    if (m_AssetsLoaded) {
        return;
    }

    if (!TTF_WasInit() && TTF_Init() < 0) {
        Logger::Error("[LoadingState] TTF_Init failed: " + std::string(TTF_GetError()));
    } else {
        const std::string bodyFont = Paths::Font("inter.otf");
        const std::string titleFont = Paths::Font("inter-bold.otf");
        if (!bodyFont.empty()) {
            m_Font = TTF_OpenFont(bodyFont.c_str(), 22);
        }
        if (!titleFont.empty()) {
            m_TitleFont = TTF_OpenFont(titleFont.c_str(), 34);
        }
    }

    m_Background.LoadGL(Paths::Image("menuDesat"));
    if (m_Background.texWidth > 0) {
        float scl = 1280.0f / static_cast<float>(m_Background.texWidth) * 1.175f;
        m_Background.SetScale(scl);
        m_Background.x = (1280.0f - m_Background.GetWidth()) * 0.5f;
        m_Background.y = (720.0f - m_Background.GetHeight()) * 0.5f;
    }

    m_AssetsLoaded = true;
}

void LoadingState::CloseFonts() {
    if (m_TitleFont) {
        TTF_CloseFont(m_TitleFont);
        m_TitleFont = nullptr;
    }
    if (m_Font) {
        TTF_CloseFont(m_Font);
        m_Font = nullptr;
    }
}

void LoadingState::Advance(OpenGLESBackend& renderer) {
    switch (m_Step) {
        case Step::LoadChart:
            m_Chart = SongChart::LoadFromFile(m_Request.chartPath);
            if (!m_Chart.has_value()) {
                m_Status = "Could not load chart";
                m_Step = Step::Failed;
                break;
            }
            if (m_Chart->stage.empty()) {
                std::string fallbackStage = StageScene::VanillaSongStage(m_Chart->songName);
                if (fallbackStage == "stage" && !m_Request.fallbackStage.empty()) {
                    fallbackStage = m_Request.fallbackStage;
                }
                m_Chart->stage = fallbackStage;
            }
            m_Progress = 0.20f;
            m_Status = "Loading stage data...";
            m_Step = Step::LoadStage;
            break;

        case Step::LoadStage:
            m_Stage = StageScene::LoadData(m_Chart->stage.empty() ? m_Request.fallbackStage : m_Chart->stage);
            if (!m_Stage.has_value()) {
                m_Status = "Could not load stage data";
                m_Step = Step::Failed;
                break;
            }
            m_Progress = 0.40f;
            m_Status = "Precaching stage images...";
            m_Step = Step::PrecacheStage;
            break;

        case Step::PrecacheStage:
            StageScene::PrecacheGL(*m_Stage);
            m_Progress = 0.65f;
            m_Status = "Precaching characters...";
            m_Step = Step::PrecacheCharacters;
            break;

        case Step::PrecacheCharacters:
            {
                Character precache;
                precache.PrecacheGL(m_Chart->player1);
                precache.PrecacheGL(m_Chart->player2);
                if (!m_Stage->hideGirlfriend) {
                    precache.PrecacheGL(m_Chart->gfVersion);
                }
            }
            m_Progress = 0.85f;
            m_Status = "Precaching music...";
            m_Step = Step::PrecacheMusic;
            break;

        case Step::PrecacheMusic:
            if (!m_Request.instPath.empty()) {
                MusicPlayer::Preload(m_Request.instPath);
            }
            if (!m_Request.playerVoicesPath.empty()) {
                VocalsPlayer::PreloadPlayer(m_Request.playerVoicesPath);
            }
            if (!m_Request.opponentVoicesPath.empty()) {
                VocalsPlayer::PreloadOpponent(m_Request.opponentVoicesPath);
            }
            m_Progress = 1.0f;
            m_Status = "Starting PlayState...";
            m_Step = Step::Done;
            break;

        case Step::Done:
            StateManager::Get().Switch(std::make_unique<PlayState>(m_Request, *m_Chart, *m_Stage));
            break;

        case Step::Failed:
            StateManager::Get().Switch(std::make_unique<FreeplayState>());
            break;
    }
}

void LoadingState::Update(float dt) {
    (void)dt;
}

void LoadingState::DrawText(OpenGLESBackend& renderer, TTF_Font* font, const std::string& text,
                            int x, int y, SDL_Color color, bool centered) const {
    RenderText::Draw(renderer, font, text, x, y, color, centered);
}

void LoadingState::Render(OpenGLESBackend& renderer) {
    if (!m_AssetsLoaded) {
        LoadAssets(renderer);
    }

    Advance(renderer);

    if (m_Background.IsLoaded()) {
        m_Background.DrawGL(renderer);
    }

    renderer.FillRect({140.0f, 210.0f, 1000.0f, 220.0f}, {0.0f, 0.0f, 0.0f, 175.0f / 255.0f});
    renderer.FillRect({200.0f, 342.0f, 880.0f, 24.0f}, {1.0f, 1.0f, 1.0f, 45.0f / 255.0f});
    renderer.FillRect({200.0f, 342.0f, 880.0f * m_Progress, 24.0f}, {1.0f, 1.0f, 1.0f, 230.0f / 255.0f});

    const SDL_Color white = { 255, 255, 255, 255 };
    const SDL_Color soft = { 220, 220, 220, 255 };
    DrawText(renderer, m_TitleFont, "Loading", 640, 248, white, true);
    DrawText(renderer, m_Font, m_Request.songName + " - " + m_Request.difficultyName, 640, 294, soft, true);
    DrawText(renderer, m_Font, m_Status, 640, 388, white, true);
}

} // namespace FNF
