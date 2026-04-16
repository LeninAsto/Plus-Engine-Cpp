/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * MainMenuState Implementation
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#include "MainMenuState.h"
#include "TitleState.h"
#include "CreditsState.h"
#include "../core/StateManager.h"
#include "../core/Logger.h"
#include "../audio/Conductor.h"
#include "../audio/MusicPlayer.h"
#include "../data/Paths.h"
#include <cmath>

namespace FNF {

static constexpr int   SCR_W         = 1280;
static constexpr int   SCR_H         = 720;
static constexpr float ITEM_SPACING   = 140.0f;
// Y start adjusted for 4 center items (mirrors: (num * 140) + 90 + (4 - count) * 70)
static constexpr float ITEM_START_Y   = 90.0f;
static constexpr float CAM_LERP       = 6.0f;
// Background scroll factor (matches scrollFactor.set(0, 0.25) in Haxe)
static constexpr float BG_SCROLL      = 0.25f;

// IDs for the side items stored at the end of the ITEM_NAMES array
static constexpr int   IDX_ACHIEVEMENTS = 4;
static constexpr int   IDX_OPTIONS      = 5;

// ---------------------------------------------------------------------------
// Enter / Exit
// ---------------------------------------------------------------------------

void MainMenuState::Enter() {
    Logger::Info("[MainMenuState] Enter");
    m_CurSelected  = 0;
    m_LastCenterIdx = 0;
    m_Accepting    = false;
    m_Confirming   = false;
    m_FlashAlpha   = 0.0f;
    m_FlickerTimer = 0.0f;
    m_FlickerVis   = true;
    m_TransTimer   = 0.0f;
    m_CamY         = 0.0f;
    m_CamTargetY   = 0.0f;
    m_AssetsLoaded = false;
}

void MainMenuState::Exit() {
    Logger::Info("[MainMenuState] Exit");
}

// ---------------------------------------------------------------------------
// LoadAssets
// ---------------------------------------------------------------------------

void MainMenuState::LoadAssets(SDL_Renderer* renderer) {
    // Backgrounds
    m_MenuBG.Load(renderer, Paths::Image("menuBG"));
    if (m_MenuBG.texWidth > 0) {
        float scl = static_cast<float>(SCR_W) / m_MenuBG.texWidth * 1.175f;
        m_MenuBG.SetScale(scl);
        m_MenuBG.x = (SCR_W - m_MenuBG.GetWidth())  * 0.5f;
        m_MenuBG.y = (SCR_H - m_MenuBG.GetHeight()) * 0.5f;
    }

    m_MenuDesat.Load(renderer, Paths::Image("menuDesat"));
    if (m_MenuDesat.texWidth > 0) {
        float scl = static_cast<float>(SCR_W) / m_MenuDesat.texWidth * 1.175f;
        m_MenuDesat.SetScale(scl);
        m_MenuDesat.x = (SCR_W - m_MenuDesat.GetWidth())  * 0.5f;
        m_MenuDesat.y = (SCR_H - m_MenuDesat.GetHeight()) * 0.5f;
    }
    m_MenuDesat.colorR = 0xfd;
    m_MenuDesat.colorG = 0x71;
    m_MenuDesat.colorB = 0x9b;
    m_MenuDesat.alpha  = 0.0f;

    // All menu items (center + side)
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        const std::string& name = ITEM_NAMES[i];
        std::string img = Paths::Image("mainmenu/menu_" + name);
        std::string xml = Paths::Xml  ("mainmenu/menu_" + name);
        if (m_Items[i].Load(renderer, img, xml)) {
            m_Items[i].AddByPrefix("idle",     name + " idle",     24, true);
            m_Items[i].AddByPrefix("selected", name + " selected", 24, true);
        } else {
            Logger::Warn("[MainMenuState] Could not load menu_" + name);
        }
    }

    if (!MusicPlayer::IsPlaying()) {
        std::string music = Paths::Music("freakyMenu");
        if (!music.empty()) {
            Conductor::SetBPM(102.0f);
            MusicPlayer::Play(music, -1, 0.7f);
        }
    }

    m_AssetsLoaded  = true;
    m_MenuBGBaseY   = m_MenuBG.y;
    LayoutItems();
    ChangeSelection(0);
}

// ---------------------------------------------------------------------------
// LayoutItems
// ---------------------------------------------------------------------------

void MainMenuState::LayoutItems() {
    // Center column: items at fixed screen positions, scale 1.0 (matches Haxe scrollFactor 0,0)
    int count = CENTER_ITEM_COUNT;
    for (int i = 0; i < count; i++) {
        m_Items[i].scaleX = m_Items[i].scaleY = 1.0f;
        m_Items[i].x = (SCR_W - m_Items[i].GetWidth()) * 0.5f;
        m_Items[i].y = ITEM_START_Y + i * ITEM_SPACING
                       + static_cast<float>(4 - count) * 70.0f;
    }

    // Achievements — bottom-left
    {
        int i = IDX_ACHIEVEMENTS;
        m_Items[i].scaleX = m_Items[i].scaleY = 1.0f;
        m_Items[i].x = 60.0f;
        m_Items[i].y = 490.0f;
    }

    // Options — bottom-right
    {
        int i = IDX_OPTIONS;
        m_Items[i].scaleX = m_Items[i].scaleY = 1.0f;
        m_Items[i].x = SCR_W - 60.0f - m_Items[i].GetWidth();
        m_Items[i].y = 490.0f;
    }
}

// ---------------------------------------------------------------------------
// ChangeSelection
// ---------------------------------------------------------------------------

void MainMenuState::ChangeSelection(int delta) {
    m_CurSelected = (m_CurSelected + delta + MENU_ITEM_COUNT) % MENU_ITEM_COUNT;

    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        bool sel = (i == m_CurSelected);
        m_Items[i].alpha = 1.0f; // all items stay fully visible (matches Haxe)
        m_Items[i].Play(sel ? "selected" : "idle");
    }

    // Re-center each center item horizontally after the animation change,
    // because idle and selected frames have different frameWidths (e.g.
    // story_mode idle=615px vs selected=796px). Mirrors centerOffsets() in Haxe.
    for (int i = 0; i < CENTER_ITEM_COUNT; i++) {
        m_Items[i].x = (SCR_W - m_Items[i].GetWidth()) * 0.5f;
    }
    // Re-anchor side items as well
    m_Items[IDX_ACHIEVEMENTS].x = 60.0f;
    m_Items[IDX_OPTIONS].x = SCR_W - 60.0f - m_Items[IDX_OPTIONS].GetWidth();

    // Camera target: midpoint Y of selected center item drives BG scroll
    if (m_CurSelected < CENTER_ITEM_COUNT) {
        float itemMidY = ITEM_START_Y + m_CurSelected * ITEM_SPACING
                         + static_cast<float>(4 - CENTER_ITEM_COUNT) * 70.0f
                         + ITEM_SPACING * 0.5f;
        m_CamTargetY = itemMidY;
    }
}

// ---------------------------------------------------------------------------
// Confirm
// ---------------------------------------------------------------------------

void MainMenuState::Confirm() {
    if (m_Accepting) return;
    m_Accepting  = true;
    m_Confirming = true;
    m_FlashAlpha = 1.0f;
    MusicPlayer::SetVolume(0.0f); // cut music on confirm (matches original)
    Logger::Info("[MainMenuState] Confirmed: " + ITEM_NAMES[m_CurSelected]);
}

// ---------------------------------------------------------------------------
// HandleEvent
// ---------------------------------------------------------------------------

void MainMenuState::HandleEvent(const SDL_Event& e) {
    if (m_Accepting) return;

    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_UP:
            case SDLK_w:
                if (m_CurSelected == IDX_ACHIEVEMENTS || m_CurSelected == IDX_OPTIONS) {
                    // From side item: return to last selected center item
                    m_CurSelected = m_LastCenterIdx;
                    ChangeSelection(0);
                } else {
                    ChangeSelection(-1);
                }
                break;
            case SDLK_DOWN:
            case SDLK_s:
                if (m_CurSelected == IDX_ACHIEVEMENTS || m_CurSelected == IDX_OPTIONS) {
                    // From side item: return to last selected center item
                    m_CurSelected = m_LastCenterIdx;
                    ChangeSelection(0);
                } else {
                    ChangeSelection(+1);
                }
                break;
            case SDLK_LEFT:
            case SDLK_a:
                if (m_CurSelected == IDX_OPTIONS) {
                    // From right side item → back to center
                    m_CurSelected = m_LastCenterIdx;
                    ChangeSelection(0);
                } else if (m_CurSelected < CENTER_ITEM_COUNT) {
                    // From center → go to left side item
                    m_LastCenterIdx = m_CurSelected;
                    m_CurSelected   = IDX_ACHIEVEMENTS;
                    ChangeSelection(0);
                }
                break;
            case SDLK_RIGHT:
            case SDLK_d:
                if (m_CurSelected == IDX_ACHIEVEMENTS) {
                    // From left side item → back to center
                    m_CurSelected = m_LastCenterIdx;
                    ChangeSelection(0);
                } else if (m_CurSelected < CENTER_ITEM_COUNT) {
                    // From center → go to right side item
                    m_LastCenterIdx = m_CurSelected;
                    m_CurSelected   = IDX_OPTIONS;
                    ChangeSelection(0);
                }
                break;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
            case SDLK_SPACE:
                Confirm();
                break;
            case SDLK_ESCAPE:
                // Mirrors Haxe: BACK goes to TitleState
                StateManager::Get().Switch(std::make_unique<TitleState>());
                break;
            default: break;
        }
    }
}

// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------

void MainMenuState::Update(float dt) {
    if (!m_AssetsLoaded) return;

    // Smooth camera lerp — only the background scrolls with it (at 0.25x)
    m_CamY = Lerp(m_CamY, m_CamTargetY, dt * CAM_LERP);
    m_MenuBG.y = m_MenuBGBaseY - m_CamY * BG_SCROLL;

    // All items: fixed screen positions, scale 1.0, just update animations
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        m_Items[i].Update(dt);
    }

    // Magenta flash fades out
    if (m_FlashAlpha > 0.0f) {
        m_FlashAlpha -= dt * 3.0f;
        if (m_FlashAlpha < 0.0f) m_FlashAlpha = 0.0f;
        m_MenuDesat.alpha = m_FlashAlpha;
    }

    // Flicker selected item during confirm, then transition
    if (m_Confirming) {
        m_FlickerTimer += dt;
        if (m_FlickerTimer >= FLICKER_SPEED) {
            m_FlickerTimer -= FLICKER_SPEED;
            m_FlickerVis = !m_FlickerVis;
            m_Items[m_CurSelected].visible = m_FlickerVis;
        }

        m_TransTimer += dt;
        if (m_TransTimer >= TRANS_DELAY) {
            const std::string& option = ITEM_NAMES[m_CurSelected];
            if (option == "credits") {
                StateManager::Get().Switch(std::make_unique<CreditsState>());
            } else {
                // Other sub-states not yet implemented — reset
                Logger::Info("[MainMenuState] TODO transition for: " + option);
                m_Confirming  = false;
                m_Accepting   = false;
                m_TransTimer  = 0.0f;
                m_FlashAlpha  = 0.0f;
                m_MenuDesat.alpha = 0.0f;
                m_Items[m_CurSelected].visible = true;
                MusicPlayer::SetVolume(0.7f);
            }
        }
    }

    MusicBeatState::Update(dt);
}

// ---------------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------------

void MainMenuState::Render(SDL_Renderer* renderer) {
    if (!m_AssetsLoaded) {
        LoadAssets(renderer);
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    m_MenuBG.Draw(renderer);

    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        m_Items[i].Draw(renderer);
    }

    if (m_MenuDesat.alpha > 0.01f) {
        m_MenuDesat.Draw(renderer);
    }
}

// ---------------------------------------------------------------------------
// BeatHit
// ---------------------------------------------------------------------------

void MainMenuState::BeatHit() {
    // No scale bump — visual beat feel comes from the selected animation itself.
}

} // namespace FNF
