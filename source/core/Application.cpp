/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Application Core Implementation
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#include "Application.h"
#include "Logger.h"
#include "MemoryManager.h"
#include "StateManager.h"
#include "../data/Paths.h"
#include "../audio/MusicPlayer.h"
#include "../audio/SoundPlayer.h"
#include "../graphics/Texture.h"
#include "../ui/TitleState.h"
#include "../ui/debug/DebugOverlay.h"
#include <cmath>
#include <filesystem>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace {

#ifdef _WIN32
std::string GetWin32ErrorMessage(DWORD errorCode) {
    LPSTR buffer = nullptr;
    const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    const DWORD length = FormatMessageA(
        flags,
        nullptr,
        errorCode,
        0,
        reinterpret_cast<LPSTR>(&buffer),
        0,
        nullptr
    );

    if (length == 0 || buffer == nullptr) {
        return "Win32 error " + std::to_string(errorCode);
    }

    std::string message(buffer, length);
    LocalFree(buffer);

    while (!message.empty() && (message.back() == '\r' || message.back() == '\n' || message.back() == ' ')) {
        message.pop_back();
    }

    return message;
}
#endif

} // namespace

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

    if (!ConfigureRuntimePlugins()) {
        Logger::Fatal("Failed to configure runtime plugins");
        return false;
    }
    
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

    ResolveTargetFPS();
    
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

void Application::ResolveTargetFPS() {
    if (m_Config.targetFPS > 0) {
        Logger::Info("Target FPS fixed by config: " + std::to_string(m_Config.targetFPS));
        return;
    }

    int resolvedFPS = 60;
    bool detected = false;

    if (m_Window) {
        const int displayIndex = SDL_GetWindowDisplayIndex(m_Window);
        if (displayIndex >= 0) {
            SDL_DisplayMode displayMode = {};
            if (SDL_GetCurrentDisplayMode(displayIndex, &displayMode) == 0 && displayMode.refresh_rate > 0) {
                resolvedFPS = displayMode.refresh_rate;
                detected = true;
            } else if (SDL_GetDesktopDisplayMode(displayIndex, &displayMode) == 0 && displayMode.refresh_rate > 0) {
                resolvedFPS = displayMode.refresh_rate;
                detected = true;
            }
        }
    }

    m_Config.targetFPS = resolvedFPS;

    if (detected) {
        Logger::Info("Target FPS auto-detected from display refresh rate: " + std::to_string(m_Config.targetFPS));
        return;
    }

    Logger::Warn("Could not detect display refresh rate, falling back to 60 FPS");
}

bool Application::ConfigureRuntimePlugins() {
#ifdef _WIN32
    wchar_t modulePath[MAX_PATH] = {};
    const DWORD pathLength = GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
    if (pathLength == 0 || pathLength >= MAX_PATH) {
        Logger::Warn("Could not resolve executable directory for runtime plugins");
        return true;
    }

    const std::filesystem::path pluginDir = std::filesystem::path(modulePath).parent_path() / "plugins";
    if (!std::filesystem::exists(pluginDir)) {
        Logger::Warn("Runtime plugin directory not found, falling back to default DLL search: " + pluginDir.string());
        return true;
    }

    if (!SetDllDirectoryW(pluginDir.c_str())) {
        Logger::Error("Failed to register runtime plugin directory '" + pluginDir.string() + "': " + GetWin32ErrorMessage(GetLastError()));
        return false;
    }

    const std::vector<std::wstring> runtimeDlls = {
        L"zlib1.dll",
        L"libpng16.dll",
        L"ogg.dll",
        L"vorbis.dll",
        L"vorbisfile.dll",
        L"vorbisenc.dll",
        L"wavpackdll.dll",
        L"SDL2.dll",
        L"SDL2_image.dll",
        L"SDL2_mixer.dll",
        L"SDL2_ttf.dll"
    };

    for (const auto& dllName : runtimeDlls) {
        const std::filesystem::path dllPath = pluginDir / dllName;
        if (!std::filesystem::exists(dllPath)) {
            continue;
        }

        HMODULE module = LoadLibraryExW(dllPath.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
        if (!module) {
            Logger::Error("Failed to load runtime plugin '" + dllPath.string() + "': " + GetWin32ErrorMessage(GetLastError()));
            return false;
        }
    }

    Logger::Info("Runtime plugins loaded from: " + pluginDir.string());
#endif

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
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F3) {
            MemoryManager::Collect(true);
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
    SoundPlayer::Shutdown();
    TextureCache::Shutdown();
    SDL_Quit();
    Logger::Info("[OK] Shutdown complete!");
    Logger::Info("========================================");

    m_Initialized = false;
}

} // namespace FNF
