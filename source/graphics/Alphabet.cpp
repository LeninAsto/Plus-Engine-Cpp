/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Alphabet Implementation
 *
 * Author: LeninAsto
 * Date: March 2026
 */

#include "Alphabet.h"
#include "Texture.h"
#include "../data/Paths.h"
#include "../core/Logger.h"
#include <SDL2/SDL_image.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cmath>
#include <algorithm>

namespace FNF {

// ---------------------------------------------------------------------------
// Static members
// ---------------------------------------------------------------------------
SDL_Texture*                            Alphabet::s_Texture     = nullptr;
std::unordered_map<char, Alphabet::CharFrame> Alphabet::s_BoldFrames;
std::unordered_map<char, Alphabet::CharFrame> Alphabet::s_NormalFrames;
bool                                    Alphabet::s_Loaded      = false;

// ---------------------------------------------------------------------------
// Atlas loading
// ---------------------------------------------------------------------------

Alphabet::CharFrame Alphabet::MakeCharFrame(
    int x, int y, int w, int h,
    int fX, int fY, int fW, int fH)
{
    CharFrame cf;
    cf.src   = { x, y, w, h };
    cf.drawW = (fW > 0) ? fW : w;
    cf.drawH = (fH > 0) ? fH : h;
    cf.offX  = -fX;   // frameX is negative in Sparrow trim data
    cf.offY  = -fY;
    return cf;
}

// Extract attribute value from a Sparrow SubTexture line
static bool GetAttr(const std::string& line, const char* key, std::string& out) {
    const std::string search = std::string(key) + "=\"";
    auto pos = line.find(search);
    if (pos == std::string::npos) return false;
    pos += search.size();
    auto end = line.find('"', pos);
    if (end == std::string::npos) return false;
    out = line.substr(pos, end - pos);
    return true;
}

static int AttrInt(const std::string& line, const char* key, int def = 0) {
    std::string val;
    if (!GetAttr(line, key, val)) return def;
    try { return std::stoi(val); } catch (...) { return def; }
}

void Alphabet::ParseAtlasXml(const std::string& xmlPath) {
    std::ifstream file(xmlPath);
    if (!file.is_open()) {
        Logger::Warn("[Alphabet] Cannot open XML: " + xmlPath);
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("<SubTexture") == std::string::npos) continue;

        std::string nameStr;
        if (!GetAttr(line, "name", nameStr)) continue;

        // Format expected: "[char] bold instance 10000"
        //              or  "[char] normal instance 10000"
        // We only want frame 10000 (the first static frame)
        bool isBold   = (nameStr.find(" bold instance 10000")   != std::string::npos);
        bool isNormal = (nameStr.find(" normal instance 10000") != std::string::npos);
        if (!isBold && !isNormal) continue;

        // The character is the first character(s) before " bold" or " normal"
        std::string prefix = isBold ? " bold instance 10000" : " normal instance 10000";
        std::string charPart = nameStr.substr(0, nameStr.find(prefix));
        if (charPart.empty() || charPart.size() > 1) continue; // skip multi-char names

        char c = charPart[0];

        int sx = AttrInt(line, "x");
        int sy = AttrInt(line, "y");
        int sw = AttrInt(line, "width");
        int sh = AttrInt(line, "height");
        int fX = AttrInt(line, "frameX");
        int fY = AttrInt(line, "frameY");
        int fW = AttrInt(line, "frameWidth");
        int fH = AttrInt(line, "frameHeight");

        CharFrame cf = MakeCharFrame(sx, sy, sw, sh, fX, fY, fW, fH);

        if (isBold)
            s_BoldFrames[c] = cf;
        else
            s_NormalFrames[c] = cf;
    }

    Logger::Info("[Alphabet] Parsed " +
        std::to_string(s_BoldFrames.size()) + " bold + " +
        std::to_string(s_NormalFrames.size()) + " normal frames");
}

bool Alphabet::LoadAtlas(SDL_Renderer* renderer) {
    if (s_Loaded) return true;

    std::string imgPath = Paths::Image("alphabet");
    std::string xmlPath = Paths::Xml("alphabet");

    if (imgPath.empty() || xmlPath.empty()) {
        Logger::Warn("[Alphabet] alphabet assets not found in Paths");
        return false;
    }

    s_Texture = TextureCache::Load(renderer, imgPath);
    if (!s_Texture) {
        Logger::Warn("[Alphabet] Failed to load alphabet.png");
        return false;
    }

    ParseAtlasXml(xmlPath);
    s_Loaded = true;
    Logger::Info("[Alphabet] Atlas loaded OK");
    return true;
}

void Alphabet::UnloadAtlas() {
    // TextureCache owns the texture, do not free here
    s_Texture = nullptr;
    s_BoldFrames.clear();
    s_NormalFrames.clear();
    s_Loaded = false;
}

// ---------------------------------------------------------------------------
// SetText
// ---------------------------------------------------------------------------

void Alphabet::SetText(const std::string& text, bool bold) {
    m_Bold = bold;
    m_Glyphs.clear();
    m_Width  = 0.0f;
    m_Height = 0.0f;

    if (!s_Loaded || !s_Texture) return;

    auto& frames = bold ? s_BoldFrames : s_NormalFrames;
    const float scale = bold ? BOLD_SCALE : 1.0f;

    // Measure line height from a typical capital letter
    float lineH = 60.0f * scale;
    {
        auto it = frames.find('A');
        if (it == frames.end()) it = frames.find('a');
        if (it != frames.end())
            lineH = static_cast<float>(it->second.drawH) * scale;
    }

    float curX  = 0.0f;
    float curY  = 0.0f;
    float maxX  = 0.0f;

    for (char raw : text) {
        if (raw == '\n') {
            maxX  = std::max(maxX, curX);
            curX  = 0.0f;
            curY += lineH * LINE_HEIGHT;
            continue;
        }

        // Bold uses uppercase, normal uses the char as-is
        char lookup = bold ? static_cast<char>(std::toupper(raw)) : raw;
        // Space
        if (raw == ' ') {
            curX += (lineH * 0.3f) + GLYPH_SPACING;
            continue;
        }

        auto it = frames.find(lookup);
        // fallback: try the other case
        if (it == frames.end() && std::isupper(lookup))
            it = frames.find(static_cast<char>(std::tolower(lookup)));
        if (it == frames.end() && std::islower(lookup))
            it = frames.find(static_cast<char>(std::toupper(lookup)));
        if (it == frames.end()) {
            curX += (lineH * 0.3f) + GLYPH_SPACING;
            continue;
        }

        const CharFrame& cf = it->second;
        GlyphEntry g;
        g.drawX   = curX + static_cast<float>(cf.offX) * scale;
        g.drawY   = curY + static_cast<float>(cf.offY) * scale;
        g.frame   = cf;
        m_Glyphs.push_back(g);

        curX += static_cast<float>(cf.drawW) * scale + GLYPH_SPACING;
    }

    maxX     = std::max(maxX, curX);
    m_Width  = maxX;
    m_Height = curY + lineH;
}

// ---------------------------------------------------------------------------
// ScreenCenterX
// ---------------------------------------------------------------------------

void Alphabet::ScreenCenterX(int screenW) {
    x = (static_cast<float>(screenW) - m_Width) * 0.5f;
}

// ---------------------------------------------------------------------------
// Draw
// ---------------------------------------------------------------------------

void Alphabet::Draw(SDL_Renderer* renderer) const {
    if (!visible || !s_Texture) return;

    const float scale = m_Bold ? BOLD_SCALE : 1.0f;
    const Uint8 a     = static_cast<Uint8>(alpha * 255.0f);

    SDL_SetTextureAlphaMod(s_Texture, a);
    SDL_SetTextureBlendMode(s_Texture, SDL_BLENDMODE_BLEND);

    for (const auto& g : m_Glyphs) {
        SDL_Rect dst = {
            static_cast<int>(x + g.drawX),
            static_cast<int>(y + g.drawY),
            static_cast<int>(g.frame.drawW * scale),
            static_cast<int>(g.frame.drawH * scale)
        };
        SDL_Rect src = g.frame.src;
        SDL_RenderCopy(renderer, s_Texture, &src, &dst);
    }

    SDL_SetTextureAlphaMod(s_Texture, 255);
}

} // namespace FNF
