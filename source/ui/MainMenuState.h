/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * MainMenuState - Main Menu Screen
 * 
 * Mirrors MainMenuState.hx:
 *   - Shows menuBG.png background with slight Y scroll on selection change
 *   - Shows menu item sprites (story_mode, freeplay, credits, options) with
 *     Sparrow atlas animations: 'idle' and 'selected'
 *   - UP/DOWN arrows navigate options
 *   - Enter/Space confirms and transitions (magenta flash + flicker)
 *   - On each beat: selected item bounces
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#pragma once

#include "../core/MusicBeatState.h"
#include "../graphics/AnimatedSprite.h"
#include "../graphics/Sprite.h"
#include <array>
#include <string>

namespace FNF {

class MainMenuState : public MusicBeatState {
public:
    // Center column: story_mode, freeplay, mods, credits
    static constexpr int CENTER_ITEM_COUNT = 4;
    // Legacy alias (total items including side ones)
    static constexpr int MENU_ITEM_COUNT   = 6;

    void Enter() override;
    void Exit()  override;
    void HandleEvent(const SDL_Event& e) override;
    void Update(float dt) override;
    void Render(SDL_Renderer* renderer) override;

protected:
    void BeatHit() override;

private:
    // -----------------------------------------------------------------------
    // Background sprites (static)
    // -----------------------------------------------------------------------
    Sprite m_MenuBG;    // menuBG.png    - purple background
    Sprite m_MenuDesat; // menuDesat.png - magenta flash overlay on confirm

    // -----------------------------------------------------------------------
    // Menu item animated sprites (Sparrow atlas)
    // -----------------------------------------------------------------------
    // Center column (indices 0-3): story_mode, freeplay, mods, credits
    // Side items (index 4 = left, 5 = right): achievements, options
    std::array<AnimatedSprite, MENU_ITEM_COUNT> m_Items;

    const std::array<std::string, MENU_ITEM_COUNT> ITEM_NAMES = {
        "story_mode", "freeplay", "mods", "credits",
        "achievements", "options"
    };

    // -----------------------------------------------------------------------
    // Selection state
    // -----------------------------------------------------------------------
    int  m_CurSelected    = 0;   // index into ITEM_NAMES
    int  m_LastCenterIdx  = 0;   // last selected center item (restored when leaving sides)
    bool m_Accepting      = false;
    bool m_Confirming     = false;

    // -----------------------------------------------------------------------
    // Flash + transition
    // -----------------------------------------------------------------------
    float m_FlashAlpha   = 0.0f;
    float m_FlickerTimer = 0.0f;
    bool  m_FlickerVis   = true;
    float m_TransTimer   = 0.0f;

    static constexpr float TRANS_DELAY   = 0.8f;
    static constexpr float FLICKER_SPEED = 0.06f;

    // -----------------------------------------------------------------------
    // Camera scroll (only affects background at 0.25x, items stay fixed)
    // -----------------------------------------------------------------------
    float m_CamY        = 0.0f;
    float m_CamTargetY  = 0.0f;
    float m_MenuBGBaseY = 0.0f; // initial centered Y of menuBG

    bool m_AssetsLoaded = false;

    // -----------------------------------------------------------------------
    // Helpers
    // -----------------------------------------------------------------------
    void LoadAssets(SDL_Renderer* renderer);
    void LayoutItems();
    void ChangeSelection(int delta);
    void Confirm();

    static float Lerp(float a, float b, float t) { return a + (b - a) * t; }
};

} // namespace FNF
