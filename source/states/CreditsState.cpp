/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * CreditsState Implementation
 *
 * Author: LeninAsto
 * Date: March 2026
 */

#include "CreditsState.h"
#include "MainMenuState.h"
#include "../backend/StateManager.h"
#include "../backend/Logger.h"
#include "../backend/MusicPlayer.h"
#include "../backend/SoundPlayer.h"
#include "../backend/Paths.h"
#include "../backend/OpenGLESBackend.h"
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
    m_Entries.push_back(MakeEntry("Lenin Asto",       "len",         "Programmer of Plus Engine",                             "https://www.youtube.com/@Lenin_Anonimo_Of", "03FC88"));
    m_Entries.push_back(MakeEntry("Legacy Odyssey",   "",            "Co-programmer of Plus Engine",                         "https://www.youtube.com/@LegacyOdyssey",    "8E07C2"));
    m_Entries.push_back(MakeEntry("DaffyToons",       "daffytoons",  "Failed Attempt at Plus Engine Programmer",              "https://github.com/DaffyToons",             "0A8451"));
    m_Entries.push_back(MakeEntry("Andres",           "slu",         "Creator and owner of several codes used",               "https://github.com/Slushi-Github",          "8FD9D1"));
    m_Entries.push_back(MakeEntry("sirthegamercoder", "sir",         "Indonesian translation and others PRs",                 "",                                          "7FDBFF"));
    m_Entries.push_back(MakeEntry("Hansuke H",        "hansu",       "Vietnamese translation and alphabet sprite",            "https://www.facebook.com/hansuke.hotaroshi", "FF6C8D"));
    m_Entries.push_back(MakeEntry("TheoDev",          "theo",        "Owner, Lead coder of Funkin Modchart",                  "https://github.com/TheoDevelops",           "FFB347"));
    m_Entries.push_back(MakeSep());
    // Mobile Porting Team
    m_Entries.push_back(MakeHeader("Mobile Porting Team"));
    m_Entries.push_back(MakeEntry("HomuHomu833",      "homura",      "Head Porter of Psych Engine",                           "https://youtube.com/@HomuHomu833",          "FFE7C0"));
    m_Entries.push_back(MakeEntry("Karim Akra",       "karim",       "Second Porter of Psych Engine",                         "https://youtube.com/@Karim0690",            "FFB4F0"));
    m_Entries.push_back(MakeEntry("Moxie",            "moxie",       "Helper of Psych Engine Mobile",                         "https://twitter.com/moxie_specalist",       "F592C4"));
    m_Entries.push_back(MakeSep());
    // Psych Team
    m_Entries.push_back(MakeHeader("Psych Team"));
    m_Entries.push_back(MakeEntry("Shadow Mario",     "shadowmario", "Main Programmer and Head of Psych Engine",              "https://ko-fi.com/shadowmario",             "444444"));
    m_Entries.push_back(MakeEntry("Riveren",          "riveren",     "Main Artist/Animator of Psych Engine",                  "https://x.com/riverennn",                  "14967B"));
    m_Entries.push_back(MakeEntry("bb-panzu",         "bb",          "Ex-Programmer of Psych Engine",                         "https://x.com/bbsub3",                     "3E813A"));
    m_Entries.push_back(MakeSep());
    // Psych Contributors
    m_Entries.push_back(MakeHeader("Psych Contributors"));
    m_Entries.push_back(MakeEntry("crowplexus",       "crowplexus",  "Linux Support, HScript Iris, Input System v3",          "https://twitter.com/IamMorwen",             "CFCFCF"));
    m_Entries.push_back(MakeEntry("Kamizeta",         "kamizeta",    "Creator of Pessy, Psych Engine's mascot",               "https://www.instagram.com/cewweey/",        "D21C11"));
    m_Entries.push_back(MakeEntry("MaxNeton",         "maxneton",    "Loading Screen Easter Egg Artist/Animator",             "https://bsky.app/profile/maxneton.bsky.social", "3C2E4E"));
    m_Entries.push_back(MakeEntry("Keoiki",           "keoiki",      "Note Splash Animations and Latin Alphabet",             "https://x.com/Keoiki_",                    "D2D2D2"));
    m_Entries.push_back(MakeEntry("SqirraRNG",        "sqirra",      "Crash Handler and Chart Editor's Waveform base code",   "https://x.com/gedehari",                   "E1843A"));
    m_Entries.push_back(MakeEntry("EliteMasterEric",  "mastereric",  "Runtime Shaders support and Other PRs",                 "https://x.com/EliteMasterEric",            "FFBD40"));
    m_Entries.push_back(MakeEntry("MAJigsaw77",       "majigsaw",    ".MP4 Video Loader Library (hxvlc)",                     "https://x.com/MAJigsaw77",                 "5F5F5F"));
    m_Entries.push_back(MakeEntry("iFlicky",          "flicky",      "Composer of Psync and Tea Time",                        "https://x.com/flicky_i",                   "9E29CF"));
    m_Entries.push_back(MakeEntry("KadeDev",          "kade",        "Fixed issues on Chart Editor and Other PRs",            "https://x.com/kade0912",                   "64A250"));
    m_Entries.push_back(MakeEntry("superpowers04",    "superpowers04", "LUA JIT Fork",                                        "https://x.com/superpowers04",              "B957ED"));
    m_Entries.push_back(MakeEntry("CheemsAndFriends", "cheems",      "Creator of FlxAnimate",                                 "https://x.com/CheemsnFriendos",            "E1E1E1"));
    m_Entries.push_back(MakeSep());
    // Funkin' Crew
    m_Entries.push_back(MakeHeader("Funkin' Crew"));
    m_Entries.push_back(MakeEntry("ninjamuffin99",    "ninjamuffin99", "Programmer of Friday Night Funkin'",                    "https://x.com/ninja_muffin99",             "CF2D2D"));
    m_Entries.push_back(MakeEntry("PhantomArcade",    "phantomarcade", "Animator of Friday Night Funkin'",                      "https://x.com/PhantomArcade3K",            "FADC45"));
    m_Entries.push_back(MakeEntry("evilsk8r",         "evilsk8r",      "Artist of Friday Night Funkin'",                        "https://x.com/evilsk8r",                   "5ABD4B"));
    m_Entries.push_back(MakeEntry("kawaisprite",      "kawaisprite",   "Composer of Friday Night Funkin'",                      "https://x.com/kawaisprite",                "378FC7"));
    m_Entries.push_back(MakeSep());
    // Discord
    m_Entries.push_back(MakeHeader("Psych Engine Discord"));
    m_Entries.push_back(MakeEntry("Join the Psych Ward!", "discord", "", "https://discord.gg/2ka77eMXDv",       "5165F6"));
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
                                OpenGLESBackend& renderer,
                                std::vector<CreditEntry>& entries,
                                std::vector<Alphabet>& labels,
                                std::vector<Sprite>& icons,
                                int& curSelected,
                                float& lerpY,
                                float& tgtR, float& tgtG, float& tgtB,
                                bool& assetsLoaded) {
    Alphabet::LoadAtlas(renderer);

    labels.clear();
    labels.resize(entries.size());
    icons.clear();
    icons.resize(entries.size());

    for (int i = 0; i < static_cast<int>(entries.size()); i++) {
        const auto& e = entries[i];
        bool bold = e.isHeader;
        labels[i].SetText(e.name, bold);

        if (!e.isHeader && !e.name.empty()) {
            std::string iconKey = e.iconName.empty() ? "credits/missing_icon" : "credits/" + e.iconName;
            std::string iconPath = Paths::Image(iconKey);
            if (iconPath.empty()) {
                iconPath = Paths::Image("credits/missing_icon");
            }
            if (!iconPath.empty()) {
                icons[i].LoadGL(iconPath);
            }
        }
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

    if (delta != 0) {
        const std::string sfx = Paths::Sound("scrollMenu");
        if (!sfx.empty()) {
            SoundPlayer::Play(sfx, 0.4f);
        }
    }

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
                if (!entry.url.empty()) {
                    const std::string sfx = Paths::Sound("confirmMenu");
                    if (!sfx.empty()) {
                        SoundPlayer::Play(sfx, 0.7f);
                    }
                    Logger::Info("[CreditsState] URL: " + entry.url);
                }
            }
            break;
        case SDLK_ESCAPE:
            m_Quitting = true;
            {
                const std::string sfx = Paths::Sound("cancelMenu");
                if (!sfx.empty()) {
                    SoundPlayer::Play(sfx, 1.0f);
                }
                StateManager::Get().SwitchWithFade(std::make_unique<MainMenuState>(), 0.7f);
            }
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

    if (MusicPlayer::IsPlaying() && MusicPlayer::GetVolume() < 0.7f) {
        MusicPlayer::SetVolume((std::min)(0.7f, MusicPlayer::GetVolume() + dt * 0.5f));
    }

    m_Background.colorR = static_cast<Uint8>(m_BgR);
    m_Background.colorG = static_cast<Uint8>(m_BgG);
    m_Background.colorB = static_cast<Uint8>(m_BgB);

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

        if (i < static_cast<int>(m_Icons.size()) && m_Icons[i].IsLoaded()) {
            m_Icons[i].visible = m_Labels[i].visible && !m_Entries[i].isHeader;
            m_Icons[i].alpha = m_Labels[i].alpha;
            m_Icons[i].x = m_Labels[i].x + m_Labels[i].GetWidth() + 10.0f;
            m_Icons[i].y = m_Labels[i].y;
        }
    }

    MusicBeatState::Update(dt);
}

// ---------------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------------

void CreditsState::Render(OpenGLESBackend& renderer) {
    if (!m_AssetsLoaded) {
        BuildEntries();
        m_Background.LoadGL(Paths::Image("menuDesat"));
        if (m_Background.texWidth > 0) {
            float scl = static_cast<float>(SCR_W) / m_Background.texWidth * 1.175f;
            m_Background.SetScale(scl);
            m_Background.x = (SCR_W - m_Background.GetWidth()) * 0.5f;
            m_Background.y = (SCR_H - m_Background.GetHeight()) * 0.5f;
        }
        LoadAssets_Credits(this, renderer, m_Entries, m_Labels,
                           m_Icons,
                           m_CurSelected, m_LerpY,
                           m_TgtR, m_TgtG, m_TgtB,
                           m_AssetsLoaded);
    }

    if (m_Background.IsLoaded()) {
        m_Background.DrawGL(renderer);
    }

    // Draw all visible labels
    for (auto& lbl : m_Labels) {
        if (lbl.visible) lbl.DrawGL(renderer);
    }

    for (auto& icon : m_Icons) {
        if (icon.visible) icon.DrawGL(renderer);
    }

    // Desc text for selected entry (drawn as small overlay at bottom)
    if (m_CurSelected < static_cast<int>(m_Entries.size())) {
        const auto& e = m_Entries[m_CurSelected];
        if (!e.desc.empty()) {
            renderer.FillRect({0.0f, static_cast<float>(SCR_H - 70), static_cast<float>(SCR_W), 70.0f},
                              {0.0f, 0.0f, 0.0f, 160.0f / 255.0f});

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
            s_Desc.DrawGL(renderer);
        }
    }
}

} // namespace FNF
