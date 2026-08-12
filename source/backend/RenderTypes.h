/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * RenderTypes - Backend-neutral render data
 */

#pragma once

#include <cstdint>

namespace FNF {

struct RenderColor {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
};

struct RenderRect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

struct TextureHandle {
    std::uint32_t id = 0;
    int width = 0;
    int height = 0;

    bool IsValid() const {
        return id != 0 && width > 0 && height > 0;
    }
};

struct SpriteDrawCommand {
    TextureHandle texture;
    RenderRect source;
    RenderRect dest;
    RenderColor color;
    float angle = 0.0f;
    bool flipX = false;
    bool flipY = false;
};

} // namespace FNF
