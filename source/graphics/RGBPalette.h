#pragma once

#include "AnimatedSprite.h"

#include <SDL2/SDL.h>
#include <algorithm>
#include <array>

namespace FNF {

class RGBPalette {
public:
    static SDL_Color LaneColor(int lane) {
        static constexpr std::array<SDL_Color, 4> kLaneColors = {{
            SDL_Color{194, 75, 153, 255},
            SDL_Color{0, 170, 255, 255},
            SDL_Color{23, 230, 65, 255},
            SDL_Color{249, 57, 63, 255}
        }};

        const int wrappedLane = ((lane % 4) + 4) % 4;
        return kLaneColors[wrappedLane];
    }

    static void ApplyLaneTint(AnimatedSprite& sprite, int lane, float brightness = 1.0f) {
        const SDL_Color color = LaneColor(lane);
        sprite.colorR = ScaleChannel(color.r, brightness);
        sprite.colorG = ScaleChannel(color.g, brightness);
        sprite.colorB = ScaleChannel(color.b, brightness);
    }

private:
    static Uint8 ScaleChannel(Uint8 value, float brightness) {
        const int scaled = static_cast<int>(static_cast<float>(value) * brightness);
        return static_cast<Uint8>((std::clamp)(scaled, 0, 255));
    }
};

} // namespace FNF