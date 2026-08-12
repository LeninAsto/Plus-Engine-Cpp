#include "RenderText.h"

namespace FNF {

void RenderText::Draw(OpenGLESBackend& renderer,
                      TTF_Font* font,
                      const std::string& text,
                      int x,
                      int y,
                      SDL_Color color,
                      bool centered) {
    if (!font || text.empty()) {
        return;
    }

    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) {
        return;
    }

    TextureHandle texture = renderer.CreateTextureFromSurface(surface, "text:" + text);
    if (texture.IsValid()) {
        SpriteDrawCommand cmd;
        cmd.texture = texture;
        cmd.source = {0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height)};
        cmd.dest = {
            static_cast<float>(centered ? x - surface->w / 2 : x),
            static_cast<float>(y),
            static_cast<float>(surface->w),
            static_cast<float>(surface->h)
        };
        cmd.color = {1.0f, 1.0f, 1.0f, 1.0f};
        renderer.DrawTexture(cmd);
        renderer.DestroyTexture(texture);
    }

    SDL_FreeSurface(surface);
}

SDL_Point RenderText::Measure(TTF_Font* font, const std::string& text) {
    SDL_Point size = {0, 0};
    if (!font || text.empty()) {
        return size;
    }

    TTF_SizeUTF8(font, text.c_str(), &size.x, &size.y);
    return size;
}

} // namespace FNF
