/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * CreditsState Implementation
 *
 * Author: LeninAsto
 * Date: March 2026
 */

#include "CreditsState.h"
#include "MainMenuState.h"
#include "../core/StateManager.h"
#include "../core/Logger.h"
#include "../audio/MusicPlayer.h"
#include <cmath>
#include <algorithm>

namespace FNF {

// ---------------------------------------------------------------------------
// BuildEntries — mirrors the defaultList in CreditsState.hx
// ---------------------------------------------------------------------------

void CreditsState::BuildEntries() {
    m_Entries.clear();

    // Plus Engine Team
    m_Entries.push_back(MakeHeader("Plus Engine Team"));
    m_Entries.push_back(MakeEntry("Lenin Asto",       "Programmer of Plus Engine",                             "https://www.youtube.com/@Lenin_Anonimo_Of", "03FC88"));
    m_Entries.push_back(MakeEntry("Legacy Odyssey",   "Co-programmer of Plus Engine",                         "https://www.youtube.com/@LegacyOdyssey",    "8E07C2"));
    m_Entries.push_back(MakeEntry("DaffyToons",        "Failed Attempt at Plus Engine Programmer",              "https://github.com/DaffyToons",             "0A8451"));
    m_Entries.push_back(MakeEntry("Andres",            "Creator and owner of several codes used",               "https://github.com/Slushi-Github",           "8FD9D1"));
    m_Entries.push_back(MakeEntry("sirthegamercoder", "Indonesian translation and others PRs",                 "",                                          "7FDBFF"));
    m_Entries.push_back(MakeEntry("Hansuke H",        "Vietnamese translation and alphabet sprite",            "https://www.facebook.com/hansuke.hotaroshi", "FF6C8D"));
    m_Entries.push_back(MakeEntry("TheoDev",           "Owner, Lead coder of Funkin Modchart",                  "https://github.com/TheoDevelops",            "FFB347"));
    m_Entries.push_back(MakeSep());
    // Mobile Porting Team
    m_Entries.push_back(MakeHeader("Mobile Porting Team"));
    m_Entries.push_back(MakeEntry("HomuHomu833",       "Head Porter of Psych Engine",                           "https://youtube.com/@HomuHomu833",           "FFE7C0"));
    m_Entries.push_back(MakeEntry("Karim Akra",        "Second Porter of Psych Engine",                         "https://youtube.com/@Karim0690",             "FFB4F0"));
    m_Entries.push_back(MakeEntry("Moxie",             "Helper of Psych Engine Mobile",                         "https://twitter.com/moxie_specalist",        "F592C4"));
    m_Entries.push_back(MakeSep());
    // Psych Team
    m_Entries.push_back(MakeHeader("Psych Team"));
    m_Entries.push_back(MakeEntry("Shadow Mario",      "Main Programmer and Head of Psych Engine",              "https://ko-fi.com/shadowmario",              "444444"));
    m_Entries.push_back(MakeEntry("Riveren",           "Main Artist/Animator of Psych Engine",                  "https://x.com/riverennn",                   "14967B"));
    m_Entries.push_back(MakeEntry("bb-panzu",          "Ex-Programmer of Psych Engine",                         "https://x.com/bbsub3",                      "3E813A"));
    m_Entries.push_back(MakeSep());
    // Psych Contributors
    m_Entries.push_back(MakeHeader("Psych Contributors"));
    m_Entries.push_back(MakeEntry("crowplexus",        "Linux Support, HScript Iris, Input System v3",          "https://twitter.com/IamMorwen",              "CFCFCF"));
    m_Entries.push_back(MakeEntry("Kamizeta",          "Creator of Pessy, Psych Engine's mascot",               "https://www.instagram.com/cewweey/",         "D21C11"));
    m_Entries.push_back(MakeEntry("MaxNeton",          "Loading Screen Easter Egg Artist/Animator",             "https://bsky.app/profile/maxneton.bsky.social","3C2E4E"));
    m_Entries.push_back(MakeEntry("Keoiki",            "Note Splash Animations and Latin Alphabet",             "https://x.com/Keoiki_",                     "D2D2D2"));
    m_Entries.push_back(MakeEntry("SqirraRNG",         "Crash Handler and Chart Editor's Waveform base code",   "https://x.com/gedehari",                    "E1843A"));
    m_Entries.push_back(MakeEntry("EliteMasterEric",   "Runtime Shaders support and Other PRs",                 "https://x.com/EliteMasterEric",             "FFBD40"));
    m_Entries.push_back(MakeEntry("MAJigsaw77",        ".MP4 Video Loader Library (hxvlc)",                     "https://x.com/MAJigsaw77",                  "5F5F5F"));
    m_Entries.push_back(MakeEntry("iFlicky",           "Composer of Psync and Tea Time",                        "https://x.com/flicky_i",                    "9E29CF"));
    m_Entries.push_back(MakeEntry("KadeDev",           "Fixed issues on Chart Editor and Other PRs",            "https://x.com/kade0912",                    "64A250"));
    m_Entries.push_back(MakeEntry("superpowers04",     "LUA JIT Fork",                                          "https://x.com/superpowers04",               "B957ED"));
    m_Entries.push_back(MakeEntry("CheemsAndFriends",  "Creator of FlxAnimate",                                 "https://x.com/CheemsnFriendos",             "E1E1E1"));
    m_Entries.push_back(MakeSep());
    // Funkin' Crew
    m_Entries.push_back(MakeHeader("Funkin' Crew"));
    m_Entries.push_back(MakeEntry("ninjamuffin99",     "Programmer of Friday Night Funkin'",                    "https://x.com/ninja_muffin99",              "CF2D2D"));
    m_Entries.push_back(MakeEntry("PhantomArcade",     "Animator of Friday Night Funkin'",                      "https://x.com/PhantomArcade3K",             "FADC45"));
    m_Entries.push_back(MakeEntry("evilsk8r",          "Artist of Friday Night Funkin'",                        "https://x.com/evilsk8r",                    "5ABD4B"));
    m_Entries.push_back(MakeEntry("kawaisprite",       "Composer of Friday Night Funkin'",                      "https://x.com/kawaisprite",                 "378FC7"));
    m_Entries.push_back(MakeSep());
    // Discord
    m_Entries.push_back(MakeHeader("Psych Engine Discord"));
    m_Entries.push_back(MakeEntry("Join the Psych Ward!", "", "https://discord.gg/2ka77eMXDv",                   "5165F6"));
}

// ---------------------------------------------------------------------------
// IsSelectable
// ---------------------------------------------------------------------------

bool CreditsState::IsSelectable(int idx) const {
    if (idx < 0 || idx >= static_cast<int>(m_Entries.size())) return false;
    return !m_Entries[idx].isHeader && !m_Entries[idx].name.empty();
}

// ---------------------------------------------------------------------------
// Enter
// ---------------------------------------------------------------------------

void CreditsState::Enter() {
    Logger::Info("[CreditsState] Enter");
    m_Quitting     = false;
    m_CurSelected  = 0;
    m_LerpY        = 0.0f;
    m_BgR = m_BgG = m_BgB = 0x22;
    m_TgtR = m_TgtG = m_TgtB = 0x22;
    m_AssetsLoaded = false;
}

void CreditsState::Exit() {
    Logger::Info("[CreditsState] Exit");
}

// ---------------------------------------------------------------------------
// LoadAssets (deferred to first Render)
// ---------------------------------------------------------------------------

static void LoadAssets_Credits(CreditsState* self,
                                SDL_Renderer* renderer,
                                std::vector<CreditEntry>& entries,
                                std::vector<Alphabet>& labels,
                                int& curSelected,
                                float& lerpY,
                                float& tgtR, float& tgtG, float& tgtB,
                                bool& assetsLoaded) {
    Alphabet::LoadAtlas(renderer);

    labels.clear();
    labels.resize(entries.size());

    for (int i = 0; i < static_cast<int>(entries.size()); i++) {
        const auto& e = entries[i];
        bool bold = e.isHeader;
        labels[i].SetText(e.name, bold);
    }

    // Find first selectable entry
    curSelected = 0;
    for (int i = 0; i < static_cast<int>(entries.size()); i++) {
        if (!entries[i].isHeader && !entries[i].name.empty()) {
            curSelected = i;
            lerpY = static_cast<float>(curSelected);
            break;
        }
    }

    // Set initial BG color
    if (curSelected < static_cast<int>(entries.size())) {
        tgtR = entries[curSelected].bgR;
        tgtG = entries[curSelected].bgG;
        tgtB = entries[curSelected].bgB;
    }

    assetsLoaded = true;
}

// ---------------------------------------------------------------------------
// ChangeSelection
// ---------------------------------------------------------------------------

void CreditsState::ChangeSelection(int delta) {
    int n = static_cast<int>(m_Entries.size());
    if (n == 0) return;

    int next = m_CurSelected;
    int step = (delta >= 0) ? 1 : -1;
    int attempts = n;
    do {
        next = (next + step + n) % n;
        attempts--;
    } while (!IsSelectable(next) && attempts > 0);

    if (!IsSelectable(next)) return;
    m_CurSelected = next;

    const auto& e = m_Entries[m_CurSelected];
    m_TgtR = static_cast<float>(e.bgR);
    m_TgtG = static_cast<float>(e.bgG);
    m_TgtB = static_cast<float>(e.bgB);

    Logger::Info("[CreditsState] Selected: " + e.name);
}

// ---------------------------------------------------------------------------
// HandleEvent
// ---------------------------------------------------------------------------

void CreditsState::HandleEvent(const SDL_Event& e) {
    if (m_Quitting) return;
    if (e.type != SDL_KEYDOWN) return;

    switch (e.key.keysym.sym) {
        case SDLK_UP:
        case SDLK_w:
            ChangeSelection(-1);
            break;
        case SDLK_DOWN:
        case SDLK_s:
            ChangeSelection(+1);
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
        case SDLK_SPACE:
            if (m_CurSelected < static_cast<int>(m_Entries.size())) {
                const auto& entry = m_Entries[m_CurSelected];
                if (!entry.url.empty())
                    Logger::Info("[CreditsState] URL: " + entry.url);
            }
            break;
        case SDLK_ESCAPE:
            m_Quitting = true;
            StateManager::Get().Switch(std::make_unique<MainMenuState>());
            break;
        default: break;
    }
}

// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------

void CreditsState::Update(float dt) {
    if (!m_AssetsLoaded) return;

    // Lerp scroll position toward selected index
    m_LerpY += (static_cast<float>(m_CurSelected) - m_LerpY) * std::min(1.0f, dt * LERP_SPD);

    // Lerp BG color
    m_BgR += (m_TgtR - m_BgR) * std::min(1.0f, dt * COLOR_SPD);
    m_BgG += (m_TgtG - m_BgG) * std::min(1.0f, dt * COLOR_SPD);
    m_BgB += (m_TgtB - m_BgB) * std::min(1.0f, dt * COLOR_SPD);

    // Update label X/Y positions to follow scroll
    int n = static_cast<int>(m_Labels.size());
    for (int i = 0; i < n; i++) {
        float relY = static_cast<float>(i) - m_LerpY;

        // Y: center of screen + relative offset
        float targetY = CENTER_Y + relY * ROW_H;
        m_Labels[i].y = targetY;

        // X: selected row roughly centered; others shift left
        if (m_Entries[i].isHeader) {
            // Headers always centered
            float lx = (SCR_W - m_Labels[i].GetWidth()) * 0.5f;
            m_Labels[i].x = lx;
        } else {
            float absRel = std::abs(relY);
            float targetX = (absRel < 0.1f)
                ? (SCR_W - m_Labels[i].GetWidth()) * 0.5f - 70.0f
                : 200.0f - 40.0f * absRel;
            m_Labels[i].x = targetX;
        }

        // Alpha
        if (!m_Entries[i].isHeader) {
            float absRel = std::abs(relY);
            m_Labels[i].alpha = (absRel < 0.1f) ? 1.0f : std::max(0.0f, 0.6f - absRel * 0.05f);
        } else {
            m_Labels[i].alpha = (std::abs(relY) < 5.0f) ? 0.8f : 0.0f;
        }

        m_Labels[i].visible = (std::abs(relY) < 6.0f);
    }

    MusicBeatState::Update(dt);
}

// ---------------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------------

void CreditsState::Render(SDL_Renderer* renderer) {
    if (!m_AssetsLoaded) {
        BuildEntries();
        LoadAssets_Credits(this, renderer, m_Entries, m_Labels,
                           m_CurSelected, m_LerpY,
                           m_TgtR, m_TgtG, m_TgtB,
                           m_AssetsLoaded);
    }

    // BG fill with lerped color
    SDL_SetRenderDrawColor(renderer,
        static_cast<Uint8>(m_BgR),
        static_cast<Uint8>(m_BgG),
        static_cast<Uint8>(m_BgB),
        255);
    SDL_RenderClear(renderer);

    // Draw all visible labels
    for (auto& lbl : m_Labels) {
        if (lbl.visible) lbl.Draw(renderer);
    }

    // Desc text for selected entry (drawn as small overlay at bottom)
    if (m_CurSelected < static_cast<int>(m_Entries.size())) {
        const auto& e = m_Entries[m_CurSelected];
        if (!e.desc.empty()) {
            // Semi-transparent black bar at bottom
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
            SDL_Rect bar = { 0, SCR_H - 70, SCR_W, 70 };
            SDL_RenderFillRect(renderer, &bar);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

            // Description as a simple Alphabet label rendered inline
            // (re-use a static Alphabet to avoid rebuilding every frame)
            static Alphabet s_Desc;
            static std::string s_LastDesc;
            if (s_LastDesc != e.desc) {
                s_Desc.SetText(e.desc, false);
                s_LastDesc = e.desc;
            }
            s_Desc.x = (SCR_W - s_Desc.GetWidth()) * 0.5f;
            s_Desc.y = static_cast<float>(SCR_H) - 60.0f;
            s_Desc.alpha = 1.0f;
            s_Desc.Draw(renderer);
        }
    }
}

} // namespace FNF
