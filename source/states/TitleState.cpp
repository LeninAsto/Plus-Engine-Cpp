/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * TitleState Implementation
 *
 * Mirrors TitleState.hx:
 *   - Beat-synchronized intro sequence using the Alphabet sprite font
 *   - skipIntro() on key press or at sickBeat 17
 *   - Logo bump + GF dance every beat
 *   - freakyMenu.ogg at 102 BPM, persists to MainMenuState
 *
 * Author: LeninAsto
 * Date: March 2026
 */

#include "TitleState.h"
#include "MainMenuState.h"
#include "../backend/StateManager.h"
#include "../backend/Logger.h"
#include "../backend/Conductor.h"
#include "../backend/MusicPlayer.h"
#include "../backend/SoundPlayer.h"
#include "../backend/Paths.h"
#include "../backend/OpenGLESBackend.h"
#include <SDL2/SDL.h>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <ctime>

namespace FNF {

// ---------------------------------------------------------------------------
// Layout constants
// ---------------------------------------------------------------------------
static constexpr int   SCR_W      = 1280;
static constexpr int   SCR_H      = 720;
static constexpr float LOGO_X     = -150.0f;
static constexpr float LOGO_Y     = -100.0f;
static constexpr float GF_X       =  512.0f;
static constexpr float GF_Y       =   40.0f;
static constexpr float ENTER_X    =  100.0f;
static constexpr float ENTER_Y    =  576.0f;
static constexpr float TITLE_BPM  =  102.0f;

static const std::vector<int> GF_DANCE_LEFT  = {15,16,17,18,19,20,21,22,23,24,25,26,27,28,29};
static const std::vector<int> GF_DANCE_RIGHT = {30, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14};

// Persists across state visits just like TitleState.initialized in Haxe
bool TitleState::s_Initialized = false;

// ---------------------------------------------------------------------------
// Intro text helpers
// ---------------------------------------------------------------------------

void TitleState::LoadWackyText() {
    m_WackyText = { "Friday", "Night Funkin" }; // fallback

    std::string path = Paths::GetRoot() + "/shared/data/introText.txt";
    std::ifstream f(path);
    if (!f.is_open()) {
        // Try alternate path
        path = Paths::GetRoot() + "/base_game/shared/data/introText.txt";
        f.open(path);
    }
    if (!f.is_open()) return;

    std::vector<std::pair<std::string,std::string>> pairs;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        auto sep = line.find("--");
        if (sep == std::string::npos) continue;
        std::string a = line.substr(0, sep);
        std::string b = line.substr(sep + 2);
        // Strip trailing \r
        if (!a.empty() && a.back() == '\r') a.pop_back();
        if (!b.empty() && b.back() == '\r') b.pop_back();
        pairs.push_back({ a, b });
    }
    if (!pairs.empty()) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        auto& p     = pairs[std::rand() % pairs.size()];
        m_WackyText = { p.first, p.second };
    }
}

static float IntroLineY(int lineIndex, float offset = 0.0f) {
    return static_cast<float>(lineIndex * 60) + 200.0f + offset;
}

void TitleState::AddCoolText(const std::vector<std::string>& lines, float yOffset) {
    DeleteCoolText();
    for (int i = 0; i < static_cast<int>(lines.size()); ++i) {
        IntroLine il;
        il.label.SetText(lines[i], true);
        il.label.ScreenCenterX(SCR_W);
        il.label.y = IntroLineY(i, yOffset);
        m_CredLines.push_back(std::move(il));
    }
}

void TitleState::AddMoreText(const std::string& text, float yOffset) {
    int idx = static_cast<int>(m_CredLines.size());
    IntroLine il;
    il.label.SetText(text, true);
    il.label.ScreenCenterX(SCR_W);
    il.label.y = IntroLineY(idx, yOffset);
    m_CredLines.push_back(std::move(il));
}

void TitleState::DeleteCoolText() {
    m_CredLines.clear();
}

void TitleState::SkipIntro() {
    if (m_SkippedIntro) return;
    m_SkippedIntro = true;
    DeleteCoolText();
    m_NewgroundsLogo.visible = false;
    m_BlackAlpha  = 0.0f; // remove black overlay
    m_DoFlashOut  = true;
    m_FlashTimer  = 0.0f;
    Logger::Info("[TitleState] Intro skipped");
}

// ---------------------------------------------------------------------------
// Enter / Exit
// ---------------------------------------------------------------------------

void TitleState::Enter() {
    Logger::Info("[TitleState] Enter");
    m_AssetsLoaded  = false;
    m_EnteredTitle  = false;
    m_TransTimer    = 0.0f;
    m_FlickerTimer  = 0.0f;
    m_FlickerVis    = true;
    m_DanceLeft     = false;
    m_SickBeats     = 0;
    m_SkippedIntro  = false;
    m_BlackAlpha    = 1.0f;
    m_DoFlashOut    = false;
    m_FlashTimer    = 0.0f;
    m_CredLines.clear();
    m_NewgroundsLogo.visible = false;
    LoadWackyText();
}

void TitleState::Exit() {
    // Keep music playing across state transitions
    Logger::Info("[TitleState] Exit");
}

// ---------------------------------------------------------------------------
// LoadAssets  (deferred – called once on first Render)
// ---------------------------------------------------------------------------

void TitleState::LoadAssets(OpenGLESBackend& renderer) {
    Alphabet::LoadAtlas(renderer);

    // Logo — no explicit scale (matches Haxe: logoBl has no setGraphicSize call)
    m_Logo.x = LOGO_X;
    m_Logo.y = LOGO_Y;
    if (m_Logo.LoadGL(Paths::Image("logoBumpin"), Paths::Xml("logoBumpin"))) {
        m_Logo.AddByPrefix("bump", "logo bumpin", 24, false);
        m_Logo.Play("bump");
    }

    // GF Dance
    m_GfDance.x = GF_X;
    m_GfDance.y = GF_Y;
    if (m_GfDance.LoadGL(Paths::Image("gfDanceTitle"), Paths::Xml("gfDanceTitle"))) {
        m_GfDance.AddByIndices("danceLeft",  "gfDance", GF_DANCE_LEFT,  24, false);
        m_GfDance.AddByIndices("danceRight", "gfDance", GF_DANCE_RIGHT, 24, false);
        m_GfDance.Play("danceRight");
    }

    // Title Enter text
    m_TitleEnter.x = ENTER_X;
    m_TitleEnter.y = ENTER_Y;
    if (m_TitleEnter.LoadGL(Paths::Image("titleEnter"), Paths::Xml("titleEnter"))) {
        m_TitleEnter.AddByPrefix("idle",  "ENTER IDLE",   24, true);
        m_TitleEnter.AddByPrefix("press", "ENTER FREEZE", 24, false);
        m_TitleEnter.Play("idle");
    }

    std::string newgroundsPath = Paths::Image("newgrounds_logo");
    if (!newgroundsPath.empty() && m_NewgroundsLogo.LoadGL(newgroundsPath)) {
        m_NewgroundsLogo.SetScale(0.8f);
        m_NewgroundsLogo.x = (SCR_W - m_NewgroundsLogo.GetWidth()) * 0.5f;
        m_NewgroundsLogo.y = static_cast<float>(SCR_H) * 0.52f;
        m_NewgroundsLogo.visible = false;
    }

    // Music
    Conductor::ClearBPMChanges();
    Conductor::SetBPM(TITLE_BPM);
    Conductor::songPosition = 0.0f;

    if (!MusicPlayer::IsPlaying()) {
        std::string musicPath = Paths::Music("freakyMenu");
        if (!musicPath.empty()) {
            MusicPlayer::Play(musicPath, -1, 0.0f);
            MusicPlayer::FadeIn(4000, 0.7f);
            Logger::Info("[TitleState] Playing freakyMenu");
        } else {
            Logger::Warn("[TitleState] freakyMenu not found");
        }
    }

    m_AssetsLoaded = true;

    // If previously visited, skip intro immediately
    if (s_Initialized) {
        SkipIntro();
    } else {
        s_Initialized = true;
    }
}

// ---------------------------------------------------------------------------
// LayoutSprites (unused helper kept for consistency)
// ---------------------------------------------------------------------------

void TitleState::LayoutSprites() {}

// ---------------------------------------------------------------------------
// HandleEvent
// ---------------------------------------------------------------------------

void TitleState::HandleEvent(const SDL_Event& e) {
    if (e.type != SDL_KEYDOWN) return;

    if (!m_SkippedIntro) {
        // Any key during intro skips it
        SkipIntro();
        return;
    }

    if (m_EnteredTitle) return; // already transitioning

    // After intro: any key to proceed to MainMenuState
    m_EnteredTitle = true;
    m_FlickerTimer = 0.0f;
    m_TransTimer   = 0.0f;
    m_TitleEnter.Play("press", true);
    const std::string sfx = Paths::Sound("confirmMenu");
    if (!sfx.empty()) {
        SoundPlayer::Play(sfx, 0.7f);
    }
    Logger::Info("[TitleState] Key pressed -> transitioning to MainMenuState");
}

// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------

void TitleState::Update(float dt) {
    if (!m_AssetsLoaded) return;

    m_Logo.Update(dt);
    m_GfDance.Update(dt);
    m_TitleEnter.Update(dt);

    MusicBeatState::Update(dt);

    // White flash fade-out after skipIntro
    if (m_DoFlashOut) {
        m_FlashTimer += dt;
        if (m_FlashTimer >= 2.0f) m_DoFlashOut = false;
    }

    // Press-Enter flicker + transition
    if (m_EnteredTitle) {
        m_FlickerTimer += dt;
        if (m_FlickerTimer >= FLICKER_INTERVAL) {
            m_FlickerTimer        -= FLICKER_INTERVAL;
            m_FlickerVis           = !m_FlickerVis;
            m_TitleEnter.visible   = m_FlickerVis;
        }
        m_TransTimer += dt;
        if (m_TransTimer >= TRANSITION_DELAY) {
            StateManager::Get().SwitchWithFade(std::make_unique<MainMenuState>(), 0.7f);
            m_EnteredTitle = false;
        }
    }
}

// ---------------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------------

void TitleState::Render(OpenGLESBackend& renderer) {
    if (!m_AssetsLoaded) {
        LoadAssets(renderer);
    }

    // Main sprites (always drawn; covered by black during intro)
    m_GfDance.DrawGL(renderer);
    m_Logo.DrawGL(renderer);
    m_TitleEnter.DrawGL(renderer);

    // Black overlay during intro
    if (!m_SkippedIntro && m_BlackAlpha > 0.0f) {
        renderer.FillRect({0.0f, 0.0f, static_cast<float>(SCR_W), static_cast<float>(SCR_H)},
                          {0.0f, 0.0f, 0.0f, m_BlackAlpha});

        // Intro alphabet lines
        for (auto& line : m_CredLines) {
            line.label.DrawGL(renderer);
        }

        m_NewgroundsLogo.DrawGL(renderer);
    }

    // White flash after skipIntro (fades over 2 seconds)
    if (m_DoFlashOut && m_FlashTimer < 2.0f) {
        float progress = m_FlashTimer / 2.0f;
        renderer.FillRect({0.0f, 0.0f, static_cast<float>(SCR_W), static_cast<float>(SCR_H)},
                          {1.0f, 1.0f, 1.0f, 1.0f - progress});
    }
}

// ---------------------------------------------------------------------------
// BeatHit
// ---------------------------------------------------------------------------

void TitleState::BeatHit() {
    // Logo bump + GF dance always
    // NOTE: The visual bump comes from the animation frames themselves (matches Haxe).
    // No manual scale pulse here.
    m_Logo.Play("bump", true);

    // Haxe logic: when danceLeft is TRUE, play 'danceRight' and vice versa.
    m_DanceLeft = !m_DanceLeft;
    m_GfDance.Play(m_DanceLeft ? "danceRight" : "danceLeft", true);

    if (m_SkippedIntro) return;

    // Intro sequence (mirrors sickBeats switch in TitleState.hx)
    m_SickBeats++;
    switch (m_SickBeats) {
        case 2:
            AddCoolText({ "Psych Engine by", " Shadow Mario" }, -30.0f);
            break;
        case 4:
            AddMoreText("Plus Engine by", 0.0f);
            AddMoreText("   Lenin Asto",  0.0f);
            break;
        case 5:
            DeleteCoolText();
            break;
        case 6:
            AddCoolText({ "Not associated", "with" }, -40.0f);
            break;
        case 8:
            AddMoreText("newgrounds", -40.0f);
            m_NewgroundsLogo.visible = true;
            break;
        case 9:
            DeleteCoolText();
            m_NewgroundsLogo.visible = false;
            break;
        case 10:
            AddCoolText({ m_WackyText[0] });
            break;
        case 12:
            AddMoreText(m_WackyText[1]);
            break;
        case 13:
            DeleteCoolText();
            break;
        case 14:
            AddMoreText("Friday");
            break;
        case 15:
            AddMoreText("Night");
            break;
        case 16:
            AddMoreText("Funkin");
            break;
        case 17:
            m_NewgroundsLogo.visible = false;
            SkipIntro();
            break;
        default:
            break;
    }
}

} // namespace FNF
