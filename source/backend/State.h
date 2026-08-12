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
#include <memory>

#include "Group.h"
#include "SubState.h"

namespace FNF {

class OpenGLESBackend;

class State : public Group {
public:
    virtual ~State() = default;

    bool persistentUpdate = false;
    bool persistentDraw   = true;

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
    void Update(float deltaTime) override { Group::Update(deltaTime); }

    /**
     * Called every frame to render.
     * The renderer is already cleared before this call.
     */
    virtual void Render(OpenGLESBackend& renderer) { Group::DrawGL(renderer); }

    /**
     * Open an overlay owned by this state. Useful for pause menus, popups,
     * game-over screens, and editor dialogs.
     */
    void OpenSubState(std::unique_ptr<SubState> subState) {
        CloseSubState();
        if (!subState) return;
        m_SubState = std::move(subState);
        m_SubState->SetParentState(this);
        m_SubState->Enter();
    }

    /** Close and destroy the current state-owned overlay, if any. */
    void CloseSubState() {
        if (!m_SubState) return;
        m_SubState->Exit();
        m_SubState.reset();
    }

    bool HasSubState() const {
        return m_SubState != nullptr && m_SubState->IsOpen();
    }

    SubState* CurrentSubState() {
        return m_SubState.get();
    }

    const SubState* CurrentSubState() const {
        return m_SubState.get();
    }

    void HandleSubStateEvent(const SDL_Event& event) {
        if (m_SubState && m_SubState->IsOpen()) {
            m_SubState->HandleEvent(event);
        }
    }

    void UpdateSubState(float deltaTime) {
        if (!m_SubState) return;

        m_SubState->Update(deltaTime);
        if (!m_SubState->IsOpen()) {
            CloseSubState();
        }
    }

    void RenderSubState(OpenGLESBackend& renderer) {
        if (m_SubState && m_SubState->IsOpen()) {
            m_SubState->Render(renderer);
        }
    }

    bool AllowsMainUpdate() const {
        return !HasSubState() || persistentUpdate;
    }

    bool AllowsMainDraw() const {
        return !HasSubState() || persistentDraw;
    }

protected:
    std::unique_ptr<SubState> m_SubState;
};

} // namespace FNF
