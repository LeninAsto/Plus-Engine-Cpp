/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * TitleState - Title Screen
 *
 * Mirrors TitleState.hx:
 *   - Beat-synchronized intro sequence with Alphabet text (sickBeats 1-17)
 *   - Shows logoBumpin, gfDanceTitle, titleEnter after skipIntro()
 *   - Plays freakyMenu.ogg at 102 BPM
 *   - On any key during intro → skipIntro()
 *   - On any key after intro  → transitions to MainMenuState with FadeTransition
 *   - On each beat → logo bumps (scale pulse)
 *
 * Author: LeninAsto
 * Date: March 2026
 */

#pragma once

#include "../backend/MusicBeatState.h"
#include "../objects/AnimatedSprite.h"
#include "../objects/Alphabet.h"
#include "../objects/Sprite.h"
#include <vector>
#include <string>

namespace FNF {

class OpenGLESBackend;

class TitleState : public MusicBeatState {
public:
    void Enter() override;
    void Exit()  override;
    void HandleEvent(const SDL_Event& e) override;
    void Update(float dt) override;
    void Render(OpenGLESBackend& renderer) override;

protected:
    void BeatHit() override;

private:
    // -----------------------------------------------------------------------
    // Animated sprites (Sparrow atlas)
    // -----------------------------------------------------------------------
    AnimatedSprite m_Logo;       // logoBumpin.png/xml  - animation: 'bump'
    AnimatedSprite m_GfDance;    // gfDanceTitle.png/xml - animations: 'danceLeft', 'danceRight'
    AnimatedSprite m_TitleEnter; // titleEnter.png/xml   - animations: 'idle', 'press'
    Sprite         m_NewgroundsLogo;

    bool m_AssetsLoaded = false;

    // -----------------------------------------------------------------------
    // Logo bump effect
    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------
    // GF dance state
    // -----------------------------------------------------------------------
    bool m_DanceLeft = false;

    // -----------------------------------------------------------------------
    // "Press Enter" flicker / transition
    // -----------------------------------------------------------------------
    bool  m_EnteredTitle = false;
    float m_FlickerTimer = 0.0f;
    bool  m_FlickerVis   = true;
    float m_TransTimer   = 0.0f;

    static constexpr float FLICKER_INTERVAL = 0.08f;
    static constexpr float TRANSITION_DELAY = 0.9f;

    // -----------------------------------------------------------------------
    // Intro sequence (mirrors sickBeats logic in TitleState.hx)
    // -----------------------------------------------------------------------
    static bool s_Initialized; // persistent across state visits

    bool m_SkippedIntro = false;
    int  m_SickBeats    = 0;    // beats counted only during intro

    // Black overlay that covers the screen during intro
    float m_BlackAlpha  = 1.0f;
    bool  m_DoFlashOut  = false;
    float m_FlashTimer  = 0.0f;

    // Active Alphabet lines (credGroup equivalent)
    struct IntroLine {
        Alphabet label;
        float    targetY = 0.0f;
    };
    std::vector<IntroLine> m_CredLines;
    std::vector<std::string> m_WackyText; // two random phrases

    // -----------------------------------------------------------------------
    // Helpers
    // -----------------------------------------------------------------------
    void LoadAssets(OpenGLESBackend& renderer);
    void LayoutSprites();
    void SkipIntro();
    void AddCoolText(const std::vector<std::string>& lines, float yOffset = 0.0f);
    void AddMoreText(const std::string& text, float yOffset = 0.0f);
    void DeleteCoolText();
    void LoadWackyText();
};

} // namespace FNF
