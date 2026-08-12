/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Alphabet - Sprite-based bitmap text renderer
 *
 * Uses the Sparrow atlas 'alphabet.png/xml' to render text character
 * by character, mirroring the Haxe Alphabet class used in the intro
 * sequence and menus.
 *
 * Frame naming convention in the atlas:
 *   "[char] bold instance 10000"   - bold style (uppercase letters, symbols)
 *   "[char] normal instance 10000" - normal style (lowercase letters)
 *
 * Only frame 10000 (the first/static frame) is used for rendering.
 * Upper case characters are used even for bold style.
 *
 * Author: LeninAsto
 * Date: March 2026
 */

#pragma once

#include "../objects/Texture.h"
#include <SDL2/SDL.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace FNF {

class OpenGLESBackend;

class Alphabet {
public:
    float x     = 0.0f;
    float y     = 0.0f;
    float alpha = 1.0f;
    bool  visible = true;

    /** Center the text horizontally on screen. */
    void ScreenCenterX(int screenW = 1280);

    /**
     * Load the shared atlas (called once before any Alphabet is used).
     * Safe to call multiple times – only loads on first call.
     */
    static bool LoadAtlas(SDL_Renderer* renderer);
    static bool LoadAtlas(OpenGLESBackend& renderer);
    static void UnloadAtlas();

    /**
     * Set the text to display.
     * @param text  UTF-8 string.  '\n' starts a new line.
     * @param bold  true = bold style (larger, used in intro), false = normal.
     */
    void SetText(const std::string& text, bool bold = true);

    /** Returns the pixel width of the current text (longest line). */
    float GetWidth()  const { return m_Width;  }
    /** Returns the pixel height of the current text (all lines). */
    float GetHeight() const { return m_Height; }

    /** Draw every character at (x + charOffset, y + lineOffset). */
    void Draw(SDL_Renderer* renderer) const;
    void DrawGL(OpenGLESBackend& renderer) const;

private:
    struct CharFrame {
        SDL_Rect src   = {};   // source rect inside atlas texture
        int      drawW = 0;    // destination draw width
        int      drawH = 0;    // destination draw height
        int      offX  = 0;   // horizontal trim offset (from frameX)
        int      offY  = 0;   // vertical   trim offset (from frameY)
    };

    struct GlyphAnim {
        std::vector<CharFrame> frames;
    };

    struct GlyphMeta {
        float offsetX = 0.0f;
        float offsetY = 0.0f;
    };

    struct LetterMeta {
        GlyphMeta normal;
        GlyphMeta bold;
        bool hasNormal = false;
        bool hasBold = false;
    };

    struct GlyphEntry {
        float     drawX = 0;   // logical x position relative to Alphabet::x
        float     drawY = 0;   // logical y position relative to Alphabet::y
        GlyphMeta meta;
        GlyphAnim anim;
    };

    static SDL_Texture*                            s_Texture;
    static TextureHandle                           s_GLTexture;
    static std::unordered_map<char, GlyphAnim>     s_BoldFrames;
    static std::unordered_map<char, GlyphAnim>     s_NormalFrames;
    static std::unordered_map<char, GlyphAnim>     s_LowercaseFrames;
    static std::unordered_map<char, GlyphAnim>     s_UppercaseFrames;
    static std::unordered_map<char, LetterMeta>    s_LetterMeta;
    static bool                                    s_Loaded;

    std::vector<GlyphEntry> m_Glyphs;
    bool  m_Bold   = true;
    float m_Width  = 0.0f;
    float m_Height = 0.0f;

    // Spacing between characters (pixels)
    static constexpr float GLYPH_SPACING = 3.0f;
    // Scale applied to bold characters to match Haxe sizing (0.9×)
    static constexpr float BOLD_SCALE    = 0.9f;
    // Line height multiplier
    static constexpr float LINE_HEIGHT   = 1.1f;

    static void ParseAtlasXml(const std::string& xmlPath);
    static void ParseAlphabetData(const std::string& jsonPath);
    static CharFrame MakeCharFrame(int x, int y, int w, int h,
                                   int fX, int fY, int fW, int fH);
    static float MeasureAnimWidth(const GlyphAnim& anim);
    static float MeasureAnimHeight(const GlyphAnim& anim);

    static constexpr float NORMAL_BASELINE = 110.0f;
    static constexpr float BOLD_BASELINE   = 70.0f;
    static constexpr float SPACE_WIDTH     = 28.0f;
    static constexpr float ROW_HEIGHT      = 85.0f;
};

} // namespace FNF
