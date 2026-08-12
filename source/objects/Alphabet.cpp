/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Alphabet Implementation
 *
 * Author: LeninAsto
 * Date: March 2026
 */

#include "Alphabet.h"
#include "Texture.h"
#include "../backend/Paths.h"
#include "../backend/JsonLoader.h"
#include "../backend/Logger.h"
#include "../backend/OpenGLESBackend.h"
#include <SDL2/SDL_image.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <cctype>

namespace FNF {

// ---------------------------------------------------------------------------
// Static members
// ---------------------------------------------------------------------------
SDL_Texture*                            Alphabet::s_Texture     = nullptr;
TextureHandle                           Alphabet::s_GLTexture   = {};
std::unordered_map<char, Alphabet::GlyphAnim> Alphabet::s_BoldFrames;
std::unordered_map<char, Alphabet::GlyphAnim> Alphabet::s_NormalFrames;
std::unordered_map<char, Alphabet::GlyphAnim> Alphabet::s_LowercaseFrames;
std::unordered_map<char, Alphabet::GlyphAnim> Alphabet::s_UppercaseFrames;
std::unordered_map<char, Alphabet::LetterMeta> Alphabet::s_LetterMeta;
bool                                    Alphabet::s_Loaded      = false;

namespace {

enum class AtlasFrameStyle {
    Bold,
    Normal,
    Lowercase,
    Uppercase
};

bool ParseFrameName(const std::string& name, std::string& glyphName, AtlasFrameStyle& style, int& instanceIndex) {
    struct StylePattern {
        AtlasFrameStyle style;
        const char* suffix;
    };

    static const StylePattern patterns[] = {
        { AtlasFrameStyle::Bold, " bold instance " },
        { AtlasFrameStyle::Normal, " normal instance " },
        { AtlasFrameStyle::Lowercase, " lowercase instance " },
        { AtlasFrameStyle::Uppercase, " uppercase instance " }
    };

    for (const auto& pattern : patterns) {
        const std::string suffix(pattern.suffix);
        const std::size_t pos = name.rfind(suffix);
        if (pos == std::string::npos) {
            continue;
        }

        glyphName = name.substr(0, pos);
        style = pattern.style;

        try {
            instanceIndex = std::stoi(name.substr(pos + suffix.size())) - 10000;
        } catch (...) {
            return false;
        }

        return instanceIndex >= 0;
    }

    return false;
}

bool ResolveGlyphChar(const std::string& glyphName, AtlasFrameStyle style, char& outChar) {
    static const std::unordered_map<std::string, char> namedGlyphs = {
        { "apostrophe", '\'' },
        { "back slash", '\\' },
        { "bullet", static_cast<char>(0x95) },
        { "comma", ',' },
        { "end quote", '"' },
        { "exclamation", '!' },
        { "forward slash", '/' },
        { "inverted exclamation", static_cast<char>(0xA1) },
        { "inverted question", static_cast<char>(0xBF) },
        { "minus", '-' },
        { "period", '.' },
        { "question", '?' },
        { "question mark", '?' },
        { "quote", '"' },
        { "semicolon", ';' },
        { "start quote", '"' }
    };

    if (glyphName.size() == 1) {
        unsigned char raw = static_cast<unsigned char>(glyphName[0]);
        switch (style) {
            case AtlasFrameStyle::Uppercase:
                outChar = static_cast<char>(std::toupper(raw));
                return true;
            case AtlasFrameStyle::Lowercase:
                outChar = static_cast<char>(std::tolower(raw));
                return true;
            default:
                outChar = glyphName[0];
                return true;
        }
    }

    auto it = namedGlyphs.find(glyphName);
    if (it == namedGlyphs.end()) {
        return false;
    }

    outChar = it->second;
    return true;
}

} // namespace

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
    auto appendFrame = [](std::unordered_map<char, GlyphAnim>& target, char glyph, int instanceIndex, const CharFrame& frame) {
        auto& anim = target[glyph];
        if (static_cast<int>(anim.frames.size()) <= instanceIndex) {
            anim.frames.resize(instanceIndex + 1);
        }
        anim.frames[instanceIndex] = frame;
    };

    while (std::getline(file, line)) {
        if (line.find("<SubTexture") == std::string::npos) continue;

        std::string nameStr;
        if (!GetAttr(line, "name", nameStr)) continue;

        std::string glyphName;
        AtlasFrameStyle style;
        int instanceIndex = 0;
        if (!ParseFrameName(nameStr, glyphName, style, instanceIndex)) continue;

        char glyph = 0;
        if (!ResolveGlyphChar(glyphName, style, glyph)) continue;

        int sx = AttrInt(line, "x");
        int sy = AttrInt(line, "y");
        int sw = AttrInt(line, "width");
        int sh = AttrInt(line, "height");
        int fX = AttrInt(line, "frameX");
        int fY = AttrInt(line, "frameY");
        int fW = AttrInt(line, "frameWidth");
        int fH = AttrInt(line, "frameHeight");

        CharFrame cf = MakeCharFrame(sx, sy, sw, sh, fX, fY, fW, fH);

        switch (style) {
            case AtlasFrameStyle::Bold:
                appendFrame(s_BoldFrames, static_cast<char>(std::tolower(static_cast<unsigned char>(glyph))), instanceIndex, cf);
                break;
            case AtlasFrameStyle::Normal:
                appendFrame(s_NormalFrames, glyph, instanceIndex, cf);
                break;
            case AtlasFrameStyle::Lowercase:
                appendFrame(s_LowercaseFrames, static_cast<char>(std::tolower(static_cast<unsigned char>(glyph))), instanceIndex, cf);
                break;
            case AtlasFrameStyle::Uppercase:
                appendFrame(s_UppercaseFrames, static_cast<char>(std::toupper(static_cast<unsigned char>(glyph))), instanceIndex, cf);
                break;
        }
    }

    Logger::Info("[Alphabet] Parsed " +
        std::to_string(s_BoldFrames.size()) + " bold + " +
        std::to_string(s_NormalFrames.size()) + " normal + " +
        std::to_string(s_LowercaseFrames.size()) + " lowercase + " +
        std::to_string(s_UppercaseFrames.size()) + " uppercase glyphs");
}

void Alphabet::ParseAlphabetData(const std::string& jsonPath) {
    s_LetterMeta.clear();

    auto jsonOpt = JsonLoader::LoadFile(jsonPath);
    if (!jsonOpt.has_value()) {
        Logger::Warn("[Alphabet] Could not load alphabet metadata: " + jsonPath);
        return;
    }

    const auto& data = jsonOpt.value();
    if (!data.contains("characters") || !data["characters"].is_object()) {
        return;
    }

    for (const auto& item : data["characters"].items()) {
        if (item.key().empty()) {
            continue;
        }

        const unsigned char raw = static_cast<unsigned char>(item.key()[0]);
        const char key = static_cast<char>(std::tolower(raw));

        LetterMeta meta;
        if (item.value().contains("normal") && item.value()["normal"].is_array() && item.value()["normal"].size() >= 2) {
            meta.normal.offsetX = item.value()["normal"][0].get<float>();
            meta.normal.offsetY = item.value()["normal"][1].get<float>();
            meta.hasNormal = true;
        }

        if (item.value().contains("bold") && item.value()["bold"].is_array() && item.value()["bold"].size() >= 2) {
            meta.bold.offsetX = item.value()["bold"][0].get<float>();
            meta.bold.offsetY = item.value()["bold"][1].get<float>();
            meta.hasBold = true;
        }

        if (meta.hasNormal || meta.hasBold) {
            s_LetterMeta[key] = meta;
        }
    }
}

bool Alphabet::LoadAtlas(SDL_Renderer* renderer) {
    if (s_Loaded) return true;

    std::string imgPath = Paths::Image("alphabet");
    std::string xmlPath = Paths::Xml("alphabet");
    std::string jsonPath = Paths::ImageJson("alphabet");

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
    if (!jsonPath.empty()) {
        ParseAlphabetData(jsonPath);
    }
    s_Loaded = true;
    Logger::Info("[Alphabet] Atlas loaded OK");
    return true;
}

bool Alphabet::LoadAtlas(OpenGLESBackend& renderer) {
    if (s_Loaded && s_GLTexture.IsValid()) return true;

    std::string imgPath = Paths::Image("alphabet");
    std::string xmlPath = Paths::Xml("alphabet");
    std::string jsonPath = Paths::ImageJson("alphabet");

    if (imgPath.empty() || xmlPath.empty()) {
        Logger::Warn("[Alphabet] alphabet assets not found in Paths");
        return false;
    }

    s_GLTexture = TextureCache::LoadGL(imgPath);
    if (!s_GLTexture.IsValid()) {
        Logger::Warn("[Alphabet] Failed to load alphabet.png");
        return false;
    }

    s_BoldFrames.clear();
    s_NormalFrames.clear();
    s_LowercaseFrames.clear();
    s_UppercaseFrames.clear();
    s_LetterMeta.clear();
    ParseAtlasXml(xmlPath);
    if (!jsonPath.empty()) {
        ParseAlphabetData(jsonPath);
    }
    s_Loaded = true;
    Logger::Info("[Alphabet] GL atlas loaded OK");
    return true;
}

void Alphabet::UnloadAtlas() {
    // TextureCache owns the texture, do not free here
    s_Texture = nullptr;
    s_GLTexture = {};
    s_BoldFrames.clear();
    s_NormalFrames.clear();
    s_LowercaseFrames.clear();
    s_UppercaseFrames.clear();
    s_LetterMeta.clear();
    s_Loaded = false;
}

float Alphabet::MeasureAnimWidth(const GlyphAnim& anim) {
    float width = 0.0f;
    for (const auto& frame : anim.frames) {
        width = std::max(width, static_cast<float>(frame.drawW));
    }
    return width;
}

float Alphabet::MeasureAnimHeight(const GlyphAnim& anim) {
    float height = 0.0f;
    for (const auto& frame : anim.frames) {
        height = std::max(height, static_cast<float>(frame.drawH));
    }
    return height;
}

// ---------------------------------------------------------------------------
// SetText
// ---------------------------------------------------------------------------

void Alphabet::SetText(const std::string& text, bool bold) {
    m_Bold = bold;
    m_Glyphs.clear();
    m_Width  = 0.0f;
    m_Height = 0.0f;

    if (!s_Loaded || (!s_Texture && !s_GLTexture.IsValid())) return;

    auto findGlyph = [](const std::unordered_map<char, GlyphAnim>& table, char key) -> const GlyphAnim* {
        auto it = table.find(key);
        if (it == table.end() || it->second.frames.empty()) {
            return nullptr;
        }
        return &it->second;
    };

    auto findMeta = [](char key) -> const LetterMeta* {
        auto it = s_LetterMeta.find(static_cast<char>(std::tolower(static_cast<unsigned char>(key))));
        if (it == s_LetterMeta.end()) {
            return nullptr;
        }
        return &it->second;
    };

    const float scale = bold ? BOLD_SCALE : 1.0f;
    const float rowStep = ROW_HEIGHT * scale;

    float lineH = 60.0f * scale;
    if (bold) {
        if (const GlyphAnim* anim = findGlyph(s_BoldFrames, 'a')) {
            lineH = MeasureAnimHeight(*anim) * scale;
        }
    } else {
        if (const GlyphAnim* anim = findGlyph(s_UppercaseFrames, 'A')) {
            lineH = MeasureAnimHeight(*anim) * scale;
        } else if (const GlyphAnim* anim = findGlyph(s_LowercaseFrames, 'a')) {
            lineH = MeasureAnimHeight(*anim) * scale;
        } else if (const GlyphAnim* anim = findGlyph(s_NormalFrames, '0')) {
            lineH = MeasureAnimHeight(*anim) * scale;
        }
    }

    float curX  = 0.0f;
    float curY  = 0.0f;
    float maxX  = 0.0f;

    for (unsigned char rawCode : text) {
        char raw = static_cast<char>(rawCode);
        if (raw == '\n') {
            maxX  = std::max(maxX, curX);
            curX  = 0.0f;
            curY += rowStep;
            continue;
        }

        if (raw == ' ') {
            curX += SPACE_WIDTH * scale;
            continue;
        }

        const GlyphAnim* anim = nullptr;
        if (bold) {
            const char lookup = static_cast<char>(std::tolower(rawCode));
            anim = findGlyph(s_BoldFrames, lookup);
            if (!anim && std::isalpha(rawCode)) {
                anim = findGlyph(s_BoldFrames, static_cast<char>(std::toupper(rawCode)));
            }
        } else {
            if (std::islower(rawCode)) {
                anim = findGlyph(s_LowercaseFrames, raw);
                if (!anim) anim = findGlyph(s_UppercaseFrames, static_cast<char>(std::toupper(rawCode)));
            } else if (std::isupper(rawCode)) {
                anim = findGlyph(s_UppercaseFrames, raw);
                if (!anim) anim = findGlyph(s_LowercaseFrames, static_cast<char>(std::tolower(rawCode)));
            }

            if (!anim) {
                anim = findGlyph(s_NormalFrames, raw);
            }
        }

        if (!anim) {
            curX += SPACE_WIDTH * scale;
            continue;
        }

        GlyphMeta glyphMeta;
        if (const LetterMeta* meta = findMeta(raw)) {
            if (bold && meta->hasBold) {
                glyphMeta = meta->bold;
            } else if (!bold && meta->hasNormal) {
                glyphMeta = meta->normal;
            }
        }

        GlyphEntry g;
        g.drawX   = curX;
        g.drawY   = curY;
        g.meta    = glyphMeta;
        g.anim    = *anim;
        m_Glyphs.push_back(g);

        const float extraKerning = bold ? 0.0f : 2.0f;
        curX += MeasureAnimWidth(*anim) * scale + (glyphMeta.offsetX + extraKerning) * scale;
    }

    maxX     = std::max(maxX, curX);
    m_Width  = maxX;
    m_Height = curY + rowStep;
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
    const Uint32 ticks = SDL_GetTicks();
    const float baseline = (m_Bold ? BOLD_BASELINE : NORMAL_BASELINE) * scale;

    SDL_SetTextureAlphaMod(s_Texture, a);
    SDL_SetTextureBlendMode(s_Texture, SDL_BLENDMODE_BLEND);

    for (const auto& g : m_Glyphs) {
        if (g.anim.frames.empty()) {
            continue;
        }

        const std::size_t frameIndex = (g.anim.frames.size() > 1)
            ? static_cast<std::size_t>(((static_cast<unsigned long long>(ticks) * 24ULL) / 1000ULL) % g.anim.frames.size())
            : 0;
        const CharFrame& frame = g.anim.frames[frameIndex];

        SDL_Rect dst = {
            static_cast<int>(x + g.drawX + static_cast<float>(frame.offX) * scale - g.meta.offsetX * scale),
            static_cast<int>(y + g.drawY + static_cast<float>(frame.offY) * scale - g.meta.offsetY * scale + baseline - frame.drawH * scale),
            static_cast<int>(frame.drawW * scale),
            static_cast<int>(frame.drawH * scale)
        };
        SDL_Rect src = frame.src;
        SDL_RenderCopy(renderer, s_Texture, &src, &dst);
    }

    SDL_SetTextureAlphaMod(s_Texture, 255);
}

void Alphabet::DrawGL(OpenGLESBackend& renderer) const {
    if (!visible || !s_GLTexture.IsValid()) return;

    const float scale = m_Bold ? BOLD_SCALE : 1.0f;
    const Uint32 ticks = SDL_GetTicks();
    const float baseline = (m_Bold ? BOLD_BASELINE : NORMAL_BASELINE) * scale;

    for (const auto& g : m_Glyphs) {
        if (g.anim.frames.empty()) {
            continue;
        }

        const std::size_t frameIndex = (g.anim.frames.size() > 1)
            ? static_cast<std::size_t>(((static_cast<unsigned long long>(ticks) * 24ULL) / 1000ULL) % g.anim.frames.size())
            : 0;
        const CharFrame& frame = g.anim.frames[frameIndex];

        SpriteDrawCommand cmd;
        cmd.texture = s_GLTexture;
        cmd.source = {
            static_cast<float>(frame.src.x),
            static_cast<float>(frame.src.y),
            static_cast<float>(frame.src.w),
            static_cast<float>(frame.src.h)
        };
        cmd.dest = {
            x + g.drawX + static_cast<float>(frame.offX) * scale - g.meta.offsetX * scale,
            y + g.drawY + static_cast<float>(frame.offY) * scale - g.meta.offsetY * scale + baseline - frame.drawH * scale,
            static_cast<float>(frame.drawW) * scale,
            static_cast<float>(frame.drawH) * scale
        };
        cmd.color = {1.0f, 1.0f, 1.0f, alpha};
        renderer.DrawTexture(cmd);
    }
}

} // namespace FNF
