/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * DebugOverlay Implementation
 *
 * 4×6 bitmap font — 96 printable ASCII glyphs (space … tilde).
 * Each glyph is encoded as 6 bytes, one per row, 4 LSBs = 4 columns.
 *
 * Author: LeninAsto
 * Date: March 2026
 */

#include "DebugOverlay.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <sstream>
#include <iomanip>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#endif

namespace FNF {

// ---------------------------------------------------------------------------
// 4×6 bitmap font data — ASCII 32 (space) … 126 (~)
// Each entry: 6 bytes, each byte = one row, bits 3..0 = columns 0..3
// Generated from a minimal 4×6 pixel set.
// ---------------------------------------------------------------------------
static const uint8_t k_Font4x6[95][6] = {
    {0x00,0x00,0x00,0x00,0x00,0x00}, // ' '
    {0x06,0x06,0x06,0x00,0x06,0x00}, // '!'
    {0x09,0x09,0x00,0x00,0x00,0x00}, // '"'
    {0x09,0x0F,0x09,0x0F,0x09,0x00}, // '#'
    {0x0E,0x0B,0x0E,0x0D,0x0E,0x00}, // '$'
    {0x09,0x02,0x04,0x09,0x00,0x00}, // '%'
    {0x06,0x09,0x06,0x0B,0x05,0x00}, // '&'
    {0x06,0x02,0x00,0x00,0x00,0x00}, // '\''
    {0x03,0x06,0x06,0x06,0x03,0x00}, // '('
    {0x0C,0x06,0x06,0x06,0x0C,0x00}, // ')'
    {0x00,0x05,0x02,0x05,0x00,0x00}, // '*'
    {0x00,0x02,0x07,0x02,0x00,0x00}, // '+'
    {0x00,0x00,0x00,0x06,0x02,0x04}, // ','
    {0x00,0x00,0x07,0x00,0x00,0x00}, // '-'
    {0x00,0x00,0x00,0x06,0x06,0x00}, // '.'
    {0x01,0x02,0x02,0x04,0x04,0x08}, // '/'
    // digits
    {0x06,0x09,0x0B,0x0D,0x06,0x00}, // '0'
    {0x02,0x06,0x02,0x02,0x07,0x00}, // '1'
    {0x06,0x09,0x02,0x04,0x0F,0x00}, // '2'
    {0x0E,0x01,0x06,0x01,0x0E,0x00}, // '3'
    {0x09,0x09,0x0F,0x01,0x01,0x00}, // '4'
    {0x0F,0x08,0x0E,0x01,0x0E,0x00}, // '5'
    {0x06,0x08,0x0E,0x09,0x06,0x00}, // '6'
    {0x0F,0x01,0x02,0x04,0x04,0x00}, // '7'
    {0x06,0x09,0x06,0x09,0x06,0x00}, // '8'
    {0x06,0x09,0x07,0x01,0x06,0x00}, // '9'
    {0x00,0x06,0x00,0x06,0x00,0x00}, // ':'
    {0x00,0x06,0x00,0x06,0x02,0x04}, // ';'
    {0x02,0x04,0x08,0x04,0x02,0x00}, // '<'
    {0x00,0x07,0x00,0x07,0x00,0x00}, // '='
    {0x04,0x02,0x01,0x02,0x04,0x00}, // '>'
    {0x06,0x01,0x02,0x00,0x02,0x00}, // '?'
    {0x06,0x0B,0x0B,0x08,0x07,0x00}, // '@'
    // uppercase
    {0x06,0x09,0x0F,0x09,0x09,0x00}, // 'A'
    {0x0E,0x09,0x0E,0x09,0x0E,0x00}, // 'B'
    {0x07,0x08,0x08,0x08,0x07,0x00}, // 'C'
    {0x0E,0x09,0x09,0x09,0x0E,0x00}, // 'D'
    {0x0F,0x08,0x0E,0x08,0x0F,0x00}, // 'E'
    {0x0F,0x08,0x0E,0x08,0x08,0x00}, // 'F'
    {0x07,0x08,0x0B,0x09,0x07,0x00}, // 'G'
    {0x09,0x09,0x0F,0x09,0x09,0x00}, // 'H'
    {0x07,0x02,0x02,0x02,0x07,0x00}, // 'I'
    {0x01,0x01,0x01,0x09,0x06,0x00}, // 'J'
    {0x09,0x0A,0x0C,0x0A,0x09,0x00}, // 'K'
    {0x08,0x08,0x08,0x08,0x0F,0x00}, // 'L'
    {0x09,0x0F,0x0F,0x09,0x09,0x00}, // 'M'
    {0x09,0x0D,0x0B,0x09,0x09,0x00}, // 'N'
    {0x06,0x09,0x09,0x09,0x06,0x00}, // 'O'
    {0x0E,0x09,0x0E,0x08,0x08,0x00}, // 'P'
    {0x06,0x09,0x09,0x0B,0x07,0x00}, // 'Q'
    {0x0E,0x09,0x0E,0x0A,0x09,0x00}, // 'R'
    {0x07,0x08,0x06,0x01,0x0E,0x00}, // 'S'
    {0x07,0x02,0x02,0x02,0x02,0x00}, // 'T'
    {0x09,0x09,0x09,0x09,0x06,0x00}, // 'U'
    {0x09,0x09,0x09,0x06,0x06,0x00}, // 'V'
    {0x09,0x09,0x0F,0x0F,0x09,0x00}, // 'W'
    {0x09,0x09,0x06,0x09,0x09,0x00}, // 'X'
    {0x09,0x09,0x06,0x02,0x02,0x00}, // 'Y'
    {0x0F,0x01,0x06,0x08,0x0F,0x00}, // 'Z'
    {0x07,0x04,0x04,0x04,0x07,0x00}, // '['
    {0x08,0x04,0x04,0x02,0x01,0x00}, // '\'
    {0x07,0x01,0x01,0x01,0x07,0x00}, // ']'
    {0x02,0x05,0x00,0x00,0x00,0x00}, // '^'
    {0x00,0x00,0x00,0x00,0x0F,0x00}, // '_'
    {0x04,0x02,0x00,0x00,0x00,0x00}, // '`'
    // lowercase
    {0x00,0x06,0x09,0x09,0x07,0x00}, // 'a'
    {0x08,0x0E,0x09,0x09,0x0E,0x00}, // 'b'
    {0x00,0x07,0x08,0x08,0x07,0x00}, // 'c'
    {0x01,0x07,0x09,0x09,0x07,0x00}, // 'd'
    {0x00,0x06,0x0F,0x08,0x07,0x00}, // 'e'
    {0x03,0x04,0x07,0x04,0x04,0x00}, // 'f'
    {0x00,0x07,0x09,0x07,0x01,0x06}, // 'g'
    {0x08,0x0E,0x09,0x09,0x09,0x00}, // 'h'
    {0x02,0x00,0x02,0x02,0x02,0x00}, // 'i'
    {0x01,0x00,0x01,0x09,0x06,0x00}, // 'j'
    {0x08,0x09,0x0A,0x0C,0x0A,0x00}, // 'k'
    {0x06,0x02,0x02,0x02,0x07,0x00}, // 'l'
    {0x00,0x0F,0x0F,0x09,0x09,0x00}, // 'm'
    {0x00,0x0E,0x09,0x09,0x09,0x00}, // 'n'
    {0x00,0x06,0x09,0x09,0x06,0x00}, // 'o'
    {0x00,0x0E,0x09,0x0E,0x08,0x00}, // 'p'
    {0x00,0x07,0x09,0x07,0x01,0x00}, // 'q'
    {0x00,0x0B,0x0C,0x08,0x08,0x00}, // 'r'
    {0x00,0x07,0x0C,0x03,0x0E,0x00}, // 's'
    {0x04,0x0F,0x04,0x04,0x03,0x00}, // 't'
    {0x00,0x09,0x09,0x09,0x07,0x00}, // 'u'
    {0x00,0x09,0x09,0x06,0x06,0x00}, // 'v'
    {0x00,0x09,0x0F,0x0F,0x06,0x00}, // 'w'
    {0x00,0x09,0x06,0x06,0x09,0x00}, // 'x'
    {0x00,0x09,0x07,0x01,0x0E,0x00}, // 'y'
    {0x00,0x0F,0x02,0x04,0x0F,0x00}, // 'z'
    {0x03,0x02,0x06,0x02,0x03,0x00}, // '{'
    {0x02,0x02,0x02,0x02,0x02,0x00}, // '|'
    {0x06,0x02,0x03,0x02,0x06,0x00}, // '}'
    {0x05,0x0A,0x00,0x00,0x00,0x00}, // '~'
};

// ---------------------------------------------------------------------------
// Init / Destroy
// ---------------------------------------------------------------------------

void DebugOverlay::Init(SDL_Renderer* renderer) {
    BuildFont(renderer);
}

void DebugOverlay::Destroy() {
    if (m_FontTex) {
        SDL_DestroyTexture(m_FontTex);
        m_FontTex = nullptr;
    }
}

// ---------------------------------------------------------------------------
// BuildFont — renders each glyph into a 4×6 grid texture (1 row, 95 glyphs)
// ---------------------------------------------------------------------------

void DebugOverlay::BuildFont(SDL_Renderer* renderer) {
    constexpr int GLYPHS  = 95;
    constexpr int TEX_W   = GLYPHS * GLYPH_W;
    constexpr int TEX_H   = GLYPH_H;

    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, TEX_W, TEX_H, 32,
                                                       SDL_PIXELFORMAT_RGBA8888);
    if (!surf) return;
    SDL_FillRect(surf, nullptr, 0x00000000); // transparent

    for (int g = 0; g < GLYPHS; g++) {
        const uint8_t* glyph = k_Font4x6[g];
        for (int row = 0; row < GLYPH_H; row++) {
            for (int col = 0; col < GLYPH_W; col++) {
                // Glyph data uses MSB = leftmost column (bit 3 = col 0, bit 0 = col 3).
                if (glyph[row] & (1 << (GLYPH_W - 1 - col))) {
                    int px = g * GLYPH_W + col;
                    int py = row;
                    uint32_t* pixels = static_cast<uint32_t*>(surf->pixels);
                    pixels[py * TEX_W + px] = 0xFFFFFFFF; // white pixel
                }
            }
        }
    }

    m_FontTex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    if (m_FontTex)
        SDL_SetTextureBlendMode(m_FontTex, SDL_BLENDMODE_BLEND);
}

// ---------------------------------------------------------------------------
// DrawChar
// ---------------------------------------------------------------------------

int DebugOverlay::DrawChar(SDL_Renderer* renderer, int px, int py, char c,
                            Uint8 r, Uint8 g, Uint8 b) const {
    if (!m_FontTex) return px + GLYPH_W * GLYPH_SCALE;

    int idx = static_cast<int>(static_cast<unsigned char>(c)) - 32;
    if (idx < 0 || idx >= 95) idx = 0; // unknown → space

    SDL_SetTextureColorMod(m_FontTex, r, g, b);

    SDL_Rect src = { idx * GLYPH_W, 0, GLYPH_W, GLYPH_H };
    SDL_Rect dst = { px, py, GLYPH_W * GLYPH_SCALE, GLYPH_H * GLYPH_SCALE };
    SDL_RenderCopy(renderer, m_FontTex, &src, &dst);

    return px + (GLYPH_W + 1) * GLYPH_SCALE; // +1 px kerning
}

void DebugOverlay::DrawString(SDL_Renderer* renderer, int px, int py,
                               const std::string& s,
                               Uint8 r, Uint8 g, Uint8 b) const {
    int cx = px;
    for (char c : s) {
        if (c == '\n') { cx = px; py += LINE_H; continue; }
        cx = DrawChar(renderer, cx, py, c, r, g, b);
    }
}

// ---------------------------------------------------------------------------
// FillRect helper
// ---------------------------------------------------------------------------

void DebugOverlay::FillRect(SDL_Renderer* rend, int x, int y, int w, int h,
                             Uint8 cr, Uint8 cg, Uint8 cb, Uint8 ca) {
    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(rend, cr, cg, cb, ca);
    SDL_Rect rect = { x, y, w, h };
    SDL_RenderFillRect(rend, &rect);
    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_NONE);
}

// ---------------------------------------------------------------------------
// Platform memory
// ---------------------------------------------------------------------------

float DebugOverlay::GetGCMemoryMB() {
#if defined(_WIN32)
    MEMORYSTATUSEX ms;
    ms.dwLength = sizeof(ms);
    GlobalMemoryStatusEx(&ms);
    // "GC memory" approximated as commit charge delta — just use heap via
    // HeapWalk would be too slow; use process private bytes as proxy instead.
    PROCESS_MEMORY_COUNTERS_EX pmc;
    pmc.cb = sizeof(pmc);
    if (GetProcessMemoryInfo(GetCurrentProcess(),
                             reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),
                             sizeof(pmc))) {
        return static_cast<float>(pmc.PrivateUsage) / (1024.0f * 1024.0f);
    }
#endif
    return 0.0f;
}

float DebugOverlay::GetTaskMemoryMB() {
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS pmc;
    pmc.cb = sizeof(pmc);
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        // WorkingSetSize = what Task Manager shows
        return static_cast<float>(pmc.WorkingSetSize) / (1024.0f * 1024.0f);
    }
#endif
    return 0.0f;
}

// ---------------------------------------------------------------------------
// CycleMode
// ---------------------------------------------------------------------------

void DebugOverlay::CycleMode() {
    switch (m_Mode) {
        case Mode::OFF:     m_Mode = Mode::MINIMAL; break;
        case Mode::MINIMAL: m_Mode = Mode::VERBOSE; break;
        case Mode::VERBOSE: m_Mode = Mode::OFF;     break;
    }
}

// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------

void DebugOverlay::Update(float dt) {
    float ms = dt * 1000.0f;
    m_LastDt = ms;

    m_FrameTimes.push_back(ms);
    if (static_cast<int>(m_FrameTimes.size()) > AVG_WINDOW)
        m_FrameTimes.pop_front();

    float sum = 0;
    for (float v : m_FrameTimes) sum += v;
    m_AvgDt = (m_FrameTimes.empty()) ? 0.0f : sum / m_FrameTimes.size();

    m_FrameCount++;
    m_FPSTimer += dt;
    if (m_FPSTimer >= 1.0f) {
        m_FPS       = m_FrameCount;
        m_FrameCount = 0;
        m_FPSTimer  -= 1.0f;

        // Update memory once per second
        float cur = GetGCMemoryMB();
        // Lerp displayed value
        if (m_DisplayMem == 0.0f) m_DisplayMem = cur;
        else m_DisplayMem += (cur - m_DisplayMem) * 0.3f;
        if (cur > m_PeakMem) m_PeakMem = cur;
    }
}

// ---------------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------------

void DebugOverlay::Render(SDL_Renderer* renderer) {
    if (m_Mode == Mode::OFF) return;
    if (!m_FontTex) return;

    // Build text
    auto fmtF = [](float v, int dec) -> std::string {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(dec) << v;
        return ss.str();
    };

    // Color: white normally, red if FPS < 30
    Uint8 fr = 255, fg = 255, fb = 255;
    if (m_FPS > 0 && m_FPS < 30) { fr = 255; fg = 60; fb = 60; }

    // First pass: measure line count to size background
    int lines = (m_Mode == Mode::MINIMAL) ? 3 : 5;
    int bgW   = (m_Mode == Mode::MINIMAL) ? 150 : 190;
    int bgH   = PAD * 2 + lines * LINE_H;

    FillRect(renderer, 0, 0, bgW + PAD * 2, bgH, 0, 0, 0, 160);

    int x = PAD, y = PAD;

    // Line 1: FPS
    DrawString(renderer, x, y, std::to_string(m_FPS) + " FPS", fr, fg, fb);
    y += LINE_H;

    // Line 2: delay ms / avg ms
    DrawString(renderer, x, y,
               fmtF(m_LastDt, 1) + " / " + fmtF(m_AvgDt, 1) + " ms",
               200, 200, 200);
    y += LINE_H;

    // Line 3: GC memory
    DrawString(renderer, x, y,
               "GC: " + fmtF(m_DisplayMem, 1) + " MB", 200, 220, 255);
    y += LINE_H;

    if (m_Mode == Mode::VERBOSE) {
        // Line 4: peak GC memory
        DrawString(renderer, x, y,
                   "Peak: " + fmtF(m_PeakMem, 1) + " MB", 180, 200, 240);
        y += LINE_H;

        // Line 5: task (working set) memory
        float task = GetTaskMemoryMB();
        DrawString(renderer, x, y,
                   "Task: " + fmtF(task, 1) + " MB", 180, 200, 240);
    }
}

} // namespace FNF
