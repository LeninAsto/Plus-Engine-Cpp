/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * CreditsState - Scrollable credits list
 *
 * Mirrors CreditsState.hx:
 *   - Alphabet-based scrollable list of credit entries
 *   - Headers are centered bold text, selectable rows are normal left-aligned
 *   - BG color lerps to the entry's associated color
 *   - DESC text shown at the bottom for the selected entry
 *   - UP/DOWN navigate; ENTER opens URL (logged); ESC goes back to MainMenuState
 *
 * Author: LeninAsto
 * Date: March 2026
 */

#pragma once

#include "../core/MusicBeatState.h"
#include "../graphics/Alphabet.h"
#include <vector>
#include <string>
#include <array>

namespace FNF {

struct CreditEntry {
    std::string name;        // display name (empty = blank separator)
    std::string desc;        // description shown at the bottom
    std::string url;         // link (logged on ENTER)
    uint8_t     bgR = 0x22; // background color
    uint8_t     bgG = 0x22;
    uint8_t     bgB = 0x22;
    bool        isHeader = false; // true if this row is a non-selectable title
};

class CreditsState : public MusicBeatState {
public:
    void Enter() override;
    void Exit()  override;
    void HandleEvent(const SDL_Event& e) override;
    void Update(float dt) override;
    void Render(SDL_Renderer* renderer) override;

private:
    static constexpr int   SCR_W      = 1280;
    static constexpr int   SCR_H      = 720;
    static constexpr float ROW_H      = 90.0f;   // vertical spacing between rows
    static constexpr float CENTER_Y   = 280.0f;  // Y of the selected row
    static constexpr float LERP_SPD   = 12.0f;   // position lerp speed
    static constexpr float COLOR_SPD  = 2.0f;    // BG color lerp speed

    // -----------------------------------------------------------------------
    // Credit data (set up in Enter)
    // -----------------------------------------------------------------------
    std::vector<CreditEntry> m_Entries;

    // -----------------------------------------------------------------------
    // Displayed Alphabet rows (one per entry)
    // -----------------------------------------------------------------------
    std::vector<Alphabet> m_Labels;

    // -----------------------------------------------------------------------
    // Selection
    // -----------------------------------------------------------------------
    int   m_CurSelected = 0;
    float m_LerpY       = 0.0f; // smooth scroll position

    // -----------------------------------------------------------------------
    // Background color (lerped)
    // -----------------------------------------------------------------------
    float m_BgR = 0x22, m_BgG = 0x22, m_BgB = 0x22;
    float m_TgtR = 0x22, m_TgtG = 0x22, m_TgtB = 0x22;

    bool m_AssetsLoaded = false;
    bool m_Quitting     = false;

    // -----------------------------------------------------------------------
    // Helpers
    // -----------------------------------------------------------------------
    void BuildEntries();
    void ChangeSelection(int delta);
    bool IsSelectable(int idx) const;

    static uint8_t ParseHexByte(const std::string& hex, int offset) {
        int val = 0;
        for (int i = 0; i < 2; i++) {
            char c = hex[offset + i];
            val <<= 4;
            if (c >= '0' && c <= '9') val |= c - '0';
            else if (c >= 'A' && c <= 'F') val |= c - 'A' + 10;
            else if (c >= 'a' && c <= 'f') val |= c - 'a' + 10;
        }
        return static_cast<uint8_t>(val);
    }

    static CreditEntry MakeEntry(const std::string& name,
                                  const std::string& desc = "",
                                  const std::string& url  = "",
                                  const std::string& hex  = "222222") {
        CreditEntry e;
        e.name     = name;
        e.desc     = desc;
        e.url      = url;
        e.isHeader = false;
        if (hex.size() >= 6) {
            e.bgR = ParseHexByte(hex, 0);
            e.bgG = ParseHexByte(hex, 2);
            e.bgB = ParseHexByte(hex, 4);
        }
        return e;
    }

    static CreditEntry MakeHeader(const std::string& name) {
        CreditEntry e;
        e.name     = name;
        e.isHeader = true;
        e.bgR = e.bgG = e.bgB = 0x22;
        return e;
    }

    static CreditEntry MakeSep() {
        CreditEntry e;
        e.isHeader = true;
        return e;
    }
};

} // namespace FNF
