/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * SubStateManager Implementation
 *
 * Author: LeninAsto
 * Date: March 2026
 */

#include "SubStateManager.h"
#include "Logger.h"

namespace FNF {

SubStateManager& SubStateManager::Get() {
    static SubStateManager instance;
    return instance;
}

void SubStateManager::OpenSubState(std::unique_ptr<SubState> sub) {
    if (m_SubState && m_SubState->IsOpen()) {
        Logger::Warn("SubStateManager: replacing an already-open sub-state");
        m_SubState->Exit();
    }
    m_SubState = std::move(sub);
    m_SubState->Enter();
}

void SubStateManager::CloseSubState() {
    if (m_SubState) {
        m_SubState->Exit();
        m_SubState.reset();
    }
}

bool SubStateManager::HasSubState() const {
    return m_SubState != nullptr && m_SubState->IsOpen();
}

SubState* SubStateManager::Current() {
    return m_SubState.get();
}

void SubStateManager::HandleEvent(const SDL_Event& e) {
    if (m_SubState) m_SubState->HandleEvent(e);
}

void SubStateManager::Update(float dt) {
    if (!m_SubState) return;
    m_SubState->Update(dt);
    // If sub-state closed itself during update, call Exit & destroy
    if (!m_SubState->IsOpen()) {
        m_SubState->Exit();
        m_SubState.reset();
    }
}

void SubStateManager::Render(SDL_Renderer* renderer) {
    if (m_SubState && m_SubState->IsOpen()) {
        m_SubState->Render(renderer);
    }
}

} // namespace FNF
