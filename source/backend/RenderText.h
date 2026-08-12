/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * RenderText - Immediate TTF text drawing for OpenGL ES
 */

#pragma once

#include "OpenGLESBackend.h"

#include <SDL2/SDL_ttf.h>
#include <string>

namespace FNF {

class RenderText {
public:
    static void Draw(OpenGLESBackend& renderer,
                     TTF_Font* font,
                     const std::string& text,
                     int x,
                     int y,
                     SDL_Color color,
                     bool centered = false);

    static SDL_Point Measure(TTF_Font* font, const std::string& text);
};

} // namespace FNF
