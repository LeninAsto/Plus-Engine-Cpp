/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * State Manager Implementation
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#include "StateManager.h"
#include "MemoryManager.h"
#include "SubStateManager.h"
#include "Logger.h"
#include "../ui/transition/FadeTransition.h"

namespace FNF {

StateManager& StateManager::Get() {
    static StateManager instance;
    return instance;
}

void StateManager::Push(std::unique_ptr<State> state) {
    m_PendingOp    = PendingOp::Push;
    m_PendingState = std::move(state);
}

void StateManager::Switch(std::unique_ptr<State> state) {
    m_PendingOp    = PendingOp::Switch;
    m_PendingState = std::move(state);
}

void StateManager::Pop() {
    m_PendingOp = PendingOp::Pop;
}

void StateManager::Clear() {
    m_PendingOp = PendingOp::Clear;
}

bool StateManager::IsEmpty() const {
    return m_Stack.empty();
}

int StateManager::StackSize() const {
    return static_cast<int>(m_Stack.size());
}

State* StateManager::Current() {
    if (m_Stack.empty()) return nullptr;
    return m_Stack.top().get();
}

void StateManager::HandleEvent(const SDL_Event& event) {
    // Block main-state input while a sub-state (transition) is active
    if (SubStateManager::Get().HasSubState()) {
        SubStateManager::Get().HandleEvent(event);
        return;
    }
    if (State* s = Current()) {
        s->HandleEvent(event);
    }
}

void StateManager::Update(float deltaTime) {
    ApplyPending();

    if (State* s = Current()) {
        s->Update(deltaTime);
    }

    // Sub-state (e.g. transition) updates on top of the main state
    SubStateManager::Get().Update(deltaTime);
}

void StateManager::Render(SDL_Renderer* renderer) {
    if (State* s = Current()) {
        s->Render(renderer);
    }
    // Sub-state renders on top of everything (transition overlay)
    SubStateManager::Get().Render(renderer);
}

void StateManager::SwitchWithFade(std::unique_ptr<State> next, float duration) {
    // Release ownership to share between the two lambdas safely via raw ptr
    State* nextRaw = next.release();

    // Phase 1 – fade OUT (cover screen with black)
    SubStateManager::Get().OpenSubState(
        std::make_unique<FadeTransition>(
            duration, false,
            [this, nextRaw, duration]() {
                // Actually switch the main state
                Switch(std::unique_ptr<State>(nextRaw));
                // Phase 2 – fade IN (reveal new state)
                SubStateManager::Get().OpenSubState(
                    std::make_unique<FadeTransition>(duration, true, nullptr)
                );
            }
        )
    );
}

void StateManager::ApplyPending() {
    if (m_PendingOp == PendingOp::None) return;

    switch (m_PendingOp) {
        case PendingOp::Push:
            m_PendingState->Enter();
            m_Stack.push(std::move(m_PendingState));
            break;

        case PendingOp::Switch:
            if (!m_Stack.empty()) {
                m_Stack.top()->Exit();
                m_Stack.pop();
                MemoryManager::Collect();
            }
            m_PendingState->Enter();
            m_Stack.push(std::move(m_PendingState));
            break;

        case PendingOp::Pop:
            if (!m_Stack.empty()) {
                m_Stack.top()->Exit();
                m_Stack.pop();
                MemoryManager::Collect();
            }
            break;

        case PendingOp::Clear:
            while (!m_Stack.empty()) {
                m_Stack.top()->Exit();
                m_Stack.pop();
            }
            MemoryManager::Collect();
            break;

        default:
            break;
    }

    m_PendingOp    = PendingOp::None;
    m_PendingState = nullptr;
}

} // namespace FNF
