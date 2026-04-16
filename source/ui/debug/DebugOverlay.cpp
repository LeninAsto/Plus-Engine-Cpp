/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * DebugOverlay Implementation
 *
 * Uses Inter from assets/fonts/inter.otf through SDL_ttf.
 *
 * Author: LeninAsto
 * Date: March 2026
 */

#include "DebugOverlay.h"
#include "../../core/Logger.h"
#include "../../data/Paths.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#endif

namespace FNF {

// ---------------------------------------------------------------------------
// Init / Destroy
// ---------------------------------------------------------------------------

void DebugOverlay::Init(SDL_Renderer* renderer) {
    (void)renderer;
    BuildFont();
}

void DebugOverlay::Destroy() {
    if (m_Font) {
        TTF_CloseFont(m_Font);
        m_Font = nullptr;
    }

    if (TTF_WasInit()) {
        TTF_Quit();
    }
}

void DebugOverlay::BuildFont() {
    if (m_Font) {
        return;
    }

    if (!TTF_WasInit() && TTF_Init() < 0) {
        Logger::Error("DebugOverlay: TTF_Init failed: " + std::string(TTF_GetError()));
        return;
    }

    const std::string fontPath = Paths::Font("inter.otf");
    if (fontPath.empty()) {
        Logger::Error("DebugOverlay: assets/fonts/inter.otf not found");
        return;
    }

    m_Font = TTF_OpenFont(fontPath.c_str(), FONT_SIZE);
    if (!m_Font) {
        Logger::Error("DebugOverlay: failed to load font '" + fontPath + "': " + std::string(TTF_GetError()));
        return;
    }

    TTF_SetFontHinting(m_Font, TTF_HINTING_LIGHT);
    Logger::Info("DebugOverlay font loaded: " + fontPath);
}

void DebugOverlay::DrawString(SDL_Renderer* renderer, int px, int py,
                               const std::string& s,
                               Uint8 r, Uint8 g, Uint8 b) const {
    if (!m_Font || s.empty()) {
        return;
    }

    SDL_Color color = { r, g, b, 255 };
    SDL_Surface* surface = TTF_RenderUTF8_Blended(m_Font, s.c_str(), color);
    if (!surface) {
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        SDL_Rect dst = { px, py, surface->w, surface->h };
        SDL_RenderCopy(renderer, texture, nullptr, &dst);
        SDL_DestroyTexture(texture);
    }

    SDL_FreeSurface(surface);
}

SDL_Point DebugOverlay::MeasureString(const std::string& s) const {
    SDL_Point size = { 0, LINE_H };
    if (!m_Font || s.empty()) {
        return size;
    }

    TTF_SizeUTF8(m_Font, s.c_str(), &size.x, &size.y);
    size.y = (std::max)(size.y, LINE_H);
    return size;
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
    if (!m_Font) return;

    // Build text
    auto fmtF = [](float v, int dec) -> std::string {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(dec) << v;
        return ss.str();
    };

    // Color: white normally, red if FPS < 30
    Uint8 fr = 255, fg = 255, fb = 255;
    if (m_FPS > 0 && m_FPS < 30) { fr = 255; fg = 60; fb = 60; }

    std::vector<std::string> lines;
    lines.push_back(std::to_string(m_FPS) + " FPS");
    lines.push_back(fmtF(m_LastDt, 1) + " / " + fmtF(m_AvgDt, 1) + " ms");
    lines.push_back("GC: " + fmtF(m_DisplayMem, 1) + " MB");
    if (m_Mode == Mode::VERBOSE) {
        lines.push_back("Peak: " + fmtF(m_PeakMem, 1) + " MB");
        lines.push_back("Task: " + fmtF(GetTaskMemoryMB(), 1) + " MB");
    }

    int maxWidth = 0;
    for (const auto& line : lines) {
        maxWidth = (std::max)(maxWidth, MeasureString(line).x);
    }

    int bgW = PAD * 2 + maxWidth;
    int bgH = PAD * 2 + static_cast<int>(lines.size()) * LINE_H;

    FillRect(renderer, 0, 0, bgW, bgH, 0, 0, 0, 160);

    int x = PAD, y = PAD;

    DrawString(renderer, x, y, lines[0], fr, fg, fb);
    y += LINE_H;

    DrawString(renderer, x, y, lines[1], 200, 200, 200);
    y += LINE_H;

    DrawString(renderer, x, y, lines[2], 200, 220, 255);
    y += LINE_H;

    if (m_Mode == Mode::VERBOSE) {
        DrawString(renderer, x, y, lines[3], 180, 200, 240);
        y += LINE_H;
        DrawString(renderer, x, y, lines[4], 180, 200, 240);
    }
}

} // namespace FNF
