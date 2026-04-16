/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * DebugOverlay - On-screen FPS / memory / delay counter
 *
 * Mirrors FPSCounter.hx (debug levels 0 and 2):
 *   F2  cycles through display modes: OFF → MINIMAL → VERBOSE → OFF
 *
 * Minimal  : FPS | delay ms | avg ms | GC mem
 * Verbose  : above + peak mem + task mem (working set)
 *
 * Rendered with a tiny 4×6 pixel bitmap font embedded as a C array,
 * no SDL_ttf dependency required.
 *
 * Author: LeninAsto
 * Date: March 2026
 */

#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <deque>
#include <cstdint>

namespace FNF {

class DebugOverlay {
public:
    enum class Mode { OFF = 0, MINIMAL, VERBOSE };

    /** Call once after renderer is created. */
    void Init(SDL_Renderer* renderer);

    /** Call once when done. */
    void Destroy();

    /**
     * Update timing metrics.
     * @param dt  Delta time in seconds for this frame.
     */
    void Update(float dt);

    /** Render overlay on top of everything. */
    void Render(SDL_Renderer* renderer);

    /** Cycle between OFF / MINIMAL / VERBOSE. */
    void CycleMode();

    Mode GetMode() const { return m_Mode; }

    /** Singleton accessor. */
    static DebugOverlay& Get() { static DebugOverlay s; return s; }

private:
    Mode  m_Mode        = Mode::MINIMAL;
    int   m_FPS         = 0;
    int   m_FrameCount  = 0;
    float m_FPSTimer    = 0.0f;

    float m_LastDt      = 0.0f;      // last frame time in ms
    float m_AvgDt       = 0.0f;      // rolling average in ms
    static constexpr int AVG_WINDOW  = 60;
    std::deque<float> m_FrameTimes;  // last N frame times (ms)

    float m_DisplayMem  = 0.0f;      // lerped GC mem (MB)
    float m_PeakMem     = 0.0f;      // peak GC mem (MB)

    SDL_Texture* m_FontTex = nullptr; // 4×6 glyph atlas

    // -----------------------------------------------------------------------
    // Font helpers (4-wide × 6-tall pixel glyphs, ASCII 32-127)
    // -----------------------------------------------------------------------
    void BuildFont(SDL_Renderer* renderer);

    /** Draw a single ASCII character at pixel pos (px, py), returns next x. */
    int DrawChar(SDL_Renderer* renderer, int px, int py, char c,
                 Uint8 r, Uint8 g, Uint8 b) const;

    /** Draw a string, returns total pixel width used. */
    void DrawString(SDL_Renderer* renderer, int px, int py,
                    const std::string& s, Uint8 r, Uint8 g, Uint8 b) const;

    /** Draw filled rect helper */
    static void FillRect(SDL_Renderer* r, int x, int y, int w, int h,
                         Uint8 cr, Uint8 cg, Uint8 cb, Uint8 ca);

    // -----------------------------------------------------------------------
    // Platform memory query
    // -----------------------------------------------------------------------
    static float GetGCMemoryMB();       // haxe GC mem ≈ heap committed memory
    static float GetTaskMemoryMB();     // process working set (like Task Manager)

    static constexpr int GLYPH_W      = 4;  // pixels per glyph column
    static constexpr int GLYPH_H      = 6;  // pixels per glyph row
    static constexpr int GLYPH_SCALE  = 2;  // draw at 2× for readability
    static constexpr int LINE_H       = (GLYPH_H + 1) * GLYPH_SCALE;
    static constexpr int PAD          = 4;  // overlay padding
};

} // namespace FNF
