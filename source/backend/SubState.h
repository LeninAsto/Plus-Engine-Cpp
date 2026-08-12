/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * SubState - Overlay State Base Class
 *
 * A SubState runs on top of the current main state without replacing it.
 * The main state keeps rendering and updating underneath.
 * Use Close() to dismiss this sub-state and return control to the main state.
 *
 * Mirrors FlxSubState from the Haxe codebase.
 *
 * Author: LeninAsto
 * Date: March 2026
 */

#pragma once

#include <SDL2/SDL.h>

namespace FNF {

class OpenGLESBackend;
class State;

class SubState {
public:
    virtual ~SubState() = default;

    /** Called once when this sub-state becomes active. */
    virtual void Enter() {}

    /** Called once before this sub-state is destroyed. */
    virtual void Exit()  {}

    /** Called every frame to handle input events. */
    virtual void HandleEvent(const SDL_Event& e) {}

    /** Called every frame with elapsed time in seconds. */
    virtual void Update(float dt) {}

    virtual void Render(OpenGLESBackend& renderer) {}

    /** Returns true while this sub-state is active. */
    bool IsOpen() const { return m_Open; }

    /** Parent state that owns this sub-state, if it was opened by a State. */
    State* ParentState() const { return m_ParentState; }

protected:
    /** Dismiss this sub-state. SubStateManager will call Exit() and destroy it. */
    void Close() { m_Open = false; }

private:
    friend class State;

    void SetParentState(State* state) { m_ParentState = state; }

    State* m_ParentState = nullptr;
    bool m_Open = true;
};

} // namespace FNF
