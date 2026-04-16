/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * SubStateManager - Manages the active overlay sub-state
 *
 * Only one sub-state may be active at a time.
 * The main state keeps rendering and updating while a sub-state is open.
 * StateManager delegates to SubStateManager after rendering the main state.
 *
 * Author: LeninAsto
 * Date: March 2026
 */

#pragma once

#include "SubState.h"
#include <memory>

namespace FNF {

class SubStateManager {
public:
    static SubStateManager& Get();

    SubStateManager(const SubStateManager&) = delete;
    SubStateManager& operator=(const SubStateManager&) = delete;

    /** Open a sub-state. Calls Enter() immediately. */
    void OpenSubState(std::unique_ptr<SubState> sub);

    /** Close the current sub-state manually. Calls Exit() and destroys it. */
    void CloseSubState();

    /** True when a sub-state is currently active. */
    bool HasSubState() const;

    /** Returns a pointer to the active sub-state (nullptr if none). */
    SubState* Current();

    // --- Called by StateManager every frame ---
    void HandleEvent(const SDL_Event& e);
    void Update(float dt);
    void Render(SDL_Renderer* renderer);

private:
    SubStateManager() = default;
    ~SubStateManager() = default;

    std::unique_ptr<SubState> m_SubState;
};

} // namespace FNF
