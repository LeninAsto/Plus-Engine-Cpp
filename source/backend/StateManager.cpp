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
#include "../substates/FadeTransition.h"

namespace FNF {

StateManager& StateManager::Get() {
    static StateManager instance;
    return instance;
}

void StateManager::Push(std::unique_ptr<State> state) {
    m_PendingOp = PendingOp::Push;
    m_PendingState = std::move(state);
}

void StateManager::Switch(std::unique_ptr<State> state) {
    m_PendingOp = PendingOp::Switch;
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
    if (SubStateManager::Get().HasSubState()) {
        SubStateManager::Get().HandleEvent(event);
        return;
    }

    if (State* state = Current()) {
        if (state->HasSubState()) {
            state->HandleSubStateEvent(event);
            return;
        }
        state->HandleEvent(event);
    }
}

void StateManager::Update(float deltaTime) {
    ApplyPending();

    if (State* state = Current()) {
        if (state->AllowsMainUpdate()) {
            state->Update(deltaTime);
        }
        state->UpdateSubState(deltaTime);
    }

    SubStateManager::Get().Update(deltaTime);
}

void StateManager::Render(OpenGLESBackend& renderer) {
    if (State* state = Current()) {
        if (state->AllowsMainDraw()) {
            state->Render(renderer);
        }
        state->RenderSubState(renderer);
    }

    SubStateManager::Get().Render(renderer);
}

void StateManager::SwitchWithFade(std::unique_ptr<State> next, float duration) {
    State* nextRaw = next.release();

    SubStateManager::Get().OpenSubState(
        std::make_unique<FadeTransition>(
            duration, false,
            [this, nextRaw, duration]() {
                Switch(std::unique_ptr<State>(nextRaw));
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
            if (m_PendingState) {
                m_PendingState->Enter();
                m_Stack.push(std::move(m_PendingState));
            }
            break;

        case PendingOp::Switch:
            if (!m_Stack.empty()) {
                m_Stack.top()->Exit();
                m_Stack.pop();
                MemoryManager::Collect();
            }
            if (m_PendingState) {
                m_PendingState->Enter();
                m_Stack.push(std::move(m_PendingState));
            }
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

        case PendingOp::None:
            break;
    }

    m_PendingOp = PendingOp::None;
    m_PendingState = nullptr;
}

} // namespace FNF
