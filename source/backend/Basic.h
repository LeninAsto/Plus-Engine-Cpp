/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Basic - Shared lifecycle flags for engine objects
 *
 * This mirrors the small but important part of FlxBasic: objects can exist,
 * update, draw, be killed, revived, and destroyed without deleting memory.
 */

#pragma once

#include <SDL2/SDL.h>

namespace FNF {

class OpenGLESBackend;

class Basic {
public:
    bool exists  = true;
    bool active  = true;
    bool visible = true;
    bool alive   = true;

    virtual ~Basic() = default;

    virtual void Update(float dt) {}
    virtual void Draw(SDL_Renderer* renderer) const {}
    virtual void DrawGL(OpenGLESBackend& backend) const {}

    virtual void Kill() {
        alive = false;
        exists = false;
    }

    virtual void Revive() {
        alive = true;
        exists = true;
    }

    virtual void Destroy() {
        alive = false;
        exists = false;
        active = false;
        visible = false;
    }

    bool ShouldUpdate() const { return exists && active; }
    bool ShouldDraw() const { return exists && visible; }
};

} // namespace FNF
