#pragma once

#include "AnimatedSprite.h"
#include "Texture.h"

#include <SDL2/SDL.h>
#include <algorithm>
#include <array>
#include <string>

namespace FNF {

class RGBPalette {
public:
    struct LanePaletteColors {
        SDL_Color redChannel;
        SDL_Color greenChannel;
        SDL_Color blueChannel;
    };

    static SDL_Color LaneColor(int lane) {
        return LanePalette(lane).redChannel;
    }

    static LanePaletteColors LanePalette(int lane) {
        static constexpr std::array<LanePaletteColors, 4> kLanePalettes = {{
            LanePaletteColors{SDL_Color{194, 75, 153, 255}, SDL_Color{255, 255, 255, 255}, SDL_Color{60, 31, 86, 255}},
            LanePaletteColors{SDL_Color{0, 255, 255, 255}, SDL_Color{255, 255, 255, 255}, SDL_Color{21, 66, 183, 255}},
            LanePaletteColors{SDL_Color{18, 250, 5, 255}, SDL_Color{255, 255, 255, 255}, SDL_Color{10, 68, 71, 255}},
            LanePaletteColors{SDL_Color{249, 57, 63, 255}, SDL_Color{255, 255, 255, 255}, SDL_Color{101, 16, 56, 255}}
        }};

        const int wrappedLane = ((lane % 4) + 4) % 4;
        return kLanePalettes[wrappedLane];
    }

    static std::string NoteAtlasCacheKey(const std::string& sourcePath, int lane) {
        const LanePaletteColors palette = LanePalette(lane);
        return sourcePath + "#rgb#" + std::to_string(lane)
             + "#" + std::to_string(palette.redChannel.r) + "," + std::to_string(palette.redChannel.g) + "," + std::to_string(palette.redChannel.b)
             + "#" + std::to_string(palette.greenChannel.r) + "," + std::to_string(palette.greenChannel.g) + "," + std::to_string(palette.greenChannel.b)
             + "#" + std::to_string(palette.blueChannel.r) + "," + std::to_string(palette.blueChannel.g) + "," + std::to_string(palette.blueChannel.b);
    }

    static SDL_Texture* LoadNoteAtlas(SDL_Renderer* renderer, const std::string& sourcePath, int lane) {
        const LanePaletteColors palette = LanePalette(lane);
        return TextureCache::LoadPaletteMapped(renderer,
                                               sourcePath,
                                               NoteAtlasCacheKey(sourcePath, lane),
                                               palette.redChannel,
                                               palette.greenChannel,
                                               palette.blueChannel,
                                               1.0f);
    }

    static void ApplyLaneTint(AnimatedSprite& sprite, int lane, float brightness = 1.0f) {
        const SDL_Color color = LaneColor(lane);
        sprite.colorR = ScaleChannel(color.r, brightness);
        sprite.colorG = ScaleChannel(color.g, brightness);
        sprite.colorB = ScaleChannel(color.b, brightness);
    }

    static void ApplyNeutralBrightness(AnimatedSprite& sprite, float brightness = 1.0f) {
        const Uint8 channel = ScaleChannel(255, brightness);
        sprite.colorR = channel;
        sprite.colorG = channel;
        sprite.colorB = channel;
    }

private:
    static Uint8 ScaleChannel(Uint8 value, float brightness) {
        const int scaled = static_cast<int>(static_cast<float>(value) * brightness);
        return static_cast<Uint8>((std::clamp)(scaled, 0, 255));
    }
};

} // namespace FNF