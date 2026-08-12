/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * State Manager
 * 
 * Manages a stack of game states.
 * The top-most state receives all events and is rendered each frame.
 * 
 * Usage:
 *   StateManager::Get().Push(std::make_unique<TitleState>());
 *   StateManager::Get().Switch(std::make_unique<PlayState>());  // replaces top
 *   StateManager::Get().Pop();                                   // go back
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#pragma once

#include "State.h"
#include "SubStateManager.h"
#include <memory>
#include <stack>
#include <functional>

namespace FNF {

class OpenGLESBackend;

class StateManager {
public:
    static StateManager& Get();

    StateManager(const StateManager&) = delete;
    StateManager& operator=(const StateManager&) = delete;

    /**
     * Push a new state on top of the stack.
     * The current state is paused but kept alive underneath.
     */
    void Push(std::unique_ptr<State> state);

    /**
     * Replace the current top state with a new one.
     * The old state's Exit() is called before the new one's Enter().
     */
    void Switch(std::unique_ptr<State> state);

    /**
     * Switch to a new state with a black fade transition.
     * Mirrors CustomFadeTransition usage in the Haxe codebase.
     * @param duration  Total time in seconds for each half of the transition.
     */
    void SwitchWithFade(std::unique_ptr<State> state, float duration = 0.7f);

    /**
     * Pop the current top state (calls Exit() and destroys it).
     * The state underneath becomes active again.
     */
    void Pop();

    /**
     * Returns true when the stack is empty (game should exit).
     */
    bool IsEmpty() const;

    /** Returns the number of states currently on the stack. */
    int StackSize() const;

    /**
     * Returns a pointer to the current active state (nullptr if empty).
     */
    State* Current();

    // --- Called by Application every frame ---
    void HandleEvent(const SDL_Event& event);
    void Update(float deltaTime);
    void Render(OpenGLESBackend& renderer);

    /**
     * Clear all states from the stack.
     */
    void Clear();

private:
    StateManager() = default;
    ~StateManager() = default;

    // Apply any pending push/switch/pop between frames to avoid
    // modifying the stack while iterating.
    void ApplyPending();

    enum class PendingOp { None, Push, Switch, Pop, Clear };

    std::stack<std::unique_ptr<State>> m_Stack;

    PendingOp m_PendingOp = PendingOp::None;
    std::unique_ptr<State> m_PendingState;
};

} // namespace FNF
