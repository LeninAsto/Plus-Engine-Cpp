/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Application Core Implementation
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#include "Application.h"
#include "Logger.h"
#include "StateManager.h"
#include "../data/Paths.h"
#include "../audio/MusicPlayer.h"
#include "../graphics/Texture.h"
#include "../ui/TitleState.h"
#include "../ui/debug/DebugOverlay.h"
#include <cmath>

namespace FNF {

Application& Application::Get() {
    static Application instance;
    return instance;
}

bool Application::Init(const ApplicationConfig& config) {
    if (m_Initialized) {
        Logger::Warn("Application already initialized");
        return true;
    }
    
    m_Config = config;
    
    Logger::Info("========================================");
    Logger::Info("  FNF Plus Engine - C++ Rewrite");
    Logger::Info("========================================");

    // Initialize asset paths (searches parent dirs if needed)
    Paths::Init("assets");
    
    if (!InitSDL()) {
        Logger::Fatal("Failed to initialize SDL");
        return false;
    }

    if (!TextureCache::Init()) {
        Logger::Fatal("Failed to initialize TextureCache (SDL_image)");
        return false;
    }

    if (!MusicPlayer::Init()) {
        Logger::Fatal("Failed to initialize MusicPlayer (SDL_mixer)");
        return false;
    }
    
    m_FrameTime = 1000.0f / static_cast<float>(m_Config.targetFPS);
    m_Initialized = true;
    
    Logger::Info("========================================");
    Logger::Info("  Application initialized successfully!");
    Logger::Info("  Press ESC or close window to exit");
    Logger::Info("========================================");

    // Push the initial game state
    StateManager::Get().Push(std::make_unique<TitleState>());

    DebugOverlay::Get().Init(m_Renderer);

    return true;
}

bool Application::InitSDL() {
    Logger::Info("Initializing SDL2...");

    SDL_SetMainReady();
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
        Logger::Error(std::string("Failed to initialize SDL: ") + SDL_GetError());
        return false;
    }
    Logger::Info("[OK] SDL2 initialized successfully!");
    
    // Create window
    Uint32 windowFlags = SDL_WINDOW_SHOWN;
    if (m_Config.fullscreen) windowFlags |= SDL_WINDOW_FULLSCREEN;
    if (m_Config.resizable) windowFlags |= SDL_WINDOW_RESIZABLE;
    
    m_Window = SDL_CreateWindow(
        m_Config.title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_Config.windowWidth,
        m_Config.windowHeight,
        windowFlags
    );
    
    if (!m_Window) {
        Logger::Error(std::string("Failed to create window: ") + SDL_GetError());
        return false;
    }
    Logger::Info("[OK] Window created: " + std::to_string(m_Config.windowWidth) + "x" + std::to_string(m_Config.windowHeight));
    
    // Create renderer
    Uint32 rendererFlags = SDL_RENDERER_ACCELERATED;
    if (m_Config.vsync) rendererFlags |= SDL_RENDERER_PRESENTVSYNC;
    
    m_Renderer = SDL_CreateRenderer(m_Window, -1, rendererFlags);
    
    if (!m_Renderer) {
        Logger::Error(std::string("Failed to create renderer: ") + SDL_GetError());
        return false;
    }
    Logger::Info("[OK] Renderer created" + std::string(m_Config.vsync ? " with VSync enabled" : ""));
    
    return true;
}

void Application::Run() {
    if (!m_Initialized) {
        Logger::Error("Cannot run application: not initialized");
        return;
    }
    
    m_Running = true;
    m_LastFrameTime = SDL_GetTicks();
    
    while (m_Running) {
        CalculateTiming();
        HandleEvents();
        Update(m_DeltaTime);
        Render();
        
        // Frame rate limiting (backup if VSync fails)
        Uint32 frameTimeMs = SDL_GetTicks() - m_LastFrameTime;
        if (frameTimeMs < m_FrameTime) {
            SDL_Delay(static_cast<Uint32>(m_FrameTime - frameTimeMs));
        }
    }
}

void Application::CalculateTiming() {
    Uint32 currentTime = SDL_GetTicks();
    m_DeltaTime = (currentTime - m_LastFrameTime) / 1000.0f;
    m_LastFrameTime = currentTime;

    DebugOverlay::Get().Update(m_DeltaTime);
}

void Application::HandleEvents() {
    SDL_Event event;
    auto& sm = StateManager::Get();
    
    while (SDL_PollEvent(&event)) {
        // Global exit conditions
        if (event.type == SDL_QUIT) {
            Logger::Info("Window close requested");
            m_Running = false;
            return;
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F2) {
            DebugOverlay::Get().CycleMode();
            return;
        }

        // Forward all events (including ESC) to the active state.
        // Each state is responsible for handling its own back/exit logic.
        // The game exits only when SDL_QUIT is received (window close / Alt+F4).
        sm.HandleEvent(event);
    }
}

void Application::Update(float deltaTime) {
    auto& sm = StateManager::Get();

    // Always update the StateManager so ApplyPending() runs and flushes
    // any state that was queued (e.g. TitleState pushed in Init).
    sm.Update(deltaTime);

    // If every state was popped out, show fallback animation and close
    if (sm.IsEmpty()) {
        m_ColorPhase += deltaTime * 2.0f;
        if (m_ColorPhase > 6.28318f) m_ColorPhase -= 6.28318f;
        m_Running = false;
    }
}

void Application::Render() {
    auto& sm = StateManager::Get();

    if (sm.IsEmpty()) {
        // Fallback: only visible briefly before the game closes
        float brightness = (std::sin(m_ColorPhase) + 1.0f) * 0.5f;
        Uint8 r = static_cast<Uint8>(40 + brightness * 40);
        Uint8 g = static_cast<Uint8>(40 + brightness * 40);
        Uint8 b = static_cast<Uint8>(70 + brightness * 100);
        SDL_SetRenderDrawColor(m_Renderer, r, g, b, 255);
        SDL_RenderClear(m_Renderer);
    } else {
        SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 255);
        SDL_RenderClear(m_Renderer);
        sm.Render(m_Renderer);
    }

    DebugOverlay::Get().Render(m_Renderer);
    SDL_RenderPresent(m_Renderer);
}

void Application::Shutdown() {
    if (!m_Initialized) {
        return;
    }
    
    Logger::Info("========================================");
    Logger::Info("Shutting down application...");
    
    if (m_Renderer) {
        SDL_DestroyRenderer(m_Renderer);
        m_Renderer = nullptr;
    }
    
    if (m_Window) {
        SDL_DestroyWindow(m_Window);
        m_Window = nullptr;
    }
    
    StateManager::Get().Clear();
    MusicPlayer::Shutdown();
    TextureCache::Shutdown();
    SDL_Quit();
    Logger::Info("[OK] Shutdown complete!");
    Logger::Info("========================================");

    m_Initialized = false;
}

} // namespace FNF
