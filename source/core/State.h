/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Base State Class
 * 
 * All game screens (TitleState, MainMenuState, PlayState, etc.) extend this.
 * The StateManager owns and drives the active state each frame.
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#pragma once

#include <SDL2/SDL.h>

namespace FNF {

class State {
public:
    virtual ~State() = default;

    /**
     * Called once when this state becomes active.
     * Load assets, initialize objects here.
     */
    virtual void Enter() {}

    /**
     * Called once before this state is destroyed.
     * Unload assets, clean up here.
     */
    virtual void Exit() {}

    /**
     * Called every frame to handle input events.
     */
    virtual void HandleEvent(const SDL_Event& event) {}

    /**
     * Called every frame with elapsed time in seconds.
     */
    virtual void Update(float deltaTime) {}

    /**
     * Called every frame to render.
     * The renderer is already cleared before this call.
     */
    virtual void Render(SDL_Renderer* renderer) {}
};

} // namespace FNF
