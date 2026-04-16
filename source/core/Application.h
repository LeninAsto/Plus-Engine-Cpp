/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Application Core Class
 * 
 * Main application class that manages the game loop, window, renderer, and timing.
 * Follows the singleton pattern for global access.
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <memory>

namespace FNF {

struct ApplicationConfig {
    std::string title = "Friday Night Funkin' Plus Engine";
    int windowWidth = 1280;
    int windowHeight = 720;
    int targetFPS = 0;
    bool vsync = true;
    bool fullscreen = false;
    bool resizable = false;
};

class Application {
public:
    /**
     * Get singleton instance
     */
    static Application& Get();
    
    /**
     * Deleted copy constructor and assignment operator
     */
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    
    /**
     * Initialize the application
     * @return true if successful
     */
    bool Init(const ApplicationConfig& config = ApplicationConfig());
    
    /**
     * Run the main game loop
     */
    void Run();
    
    /**
     * Shutdown the application
     */
    void Shutdown();
    
    /**
     * Request application to close
     */
    void Close() { m_Running = false; }
    
    /**
     * Getters
     */
    SDL_Window* GetWindow() const { return m_Window; }
    SDL_Renderer* GetRenderer() const { return m_Renderer; }
    float GetDeltaTime() const { return m_DeltaTime; }
    float GetFPS() const { return m_CurrentFPS; }
    int GetWindowWidth() const { return m_Config.windowWidth; }
    int GetWindowHeight() const { return m_Config.windowHeight; }
    bool IsRunning() const { return m_Running; }
    
private:
    Application() = default;
    ~Application() = default;
    
    /**
     * Configure runtime DLL search paths for bundled plugins
     */
    bool ConfigureRuntimePlugins();

    /**
     * Resolve target FPS from the active display refresh rate.
     */
    void ResolveTargetFPS();

    /**
     * Initialize SDL2 and create window/renderer
     */
    bool InitSDL();
    
    /**
     * Handle SDL events
     */
    void HandleEvents();
    
    /**
     * Update game logic
     */
    void Update(float deltaTime);
    
    /**
     * Render the game
     */
    void Render();
    
    /**
     * Calculate FPS and delta time
     */
    void CalculateTiming();
    
private:
    // Configuration
    ApplicationConfig m_Config;
    
    // SDL objects
    SDL_Window* m_Window = nullptr;
    SDL_Renderer* m_Renderer = nullptr;
    
    // State
    bool m_Running = false;
    bool m_Initialized = false;
    
    // Timing
    Uint32 m_LastFrameTime = 0;
    float m_DeltaTime = 0.0f;
    float m_FrameTime = 0.0f;
    
    // FPS counter
    int m_FrameCount = 0;
    float m_FPSTimer = 0.0f;
    float m_CurrentFPS = 0.0f;
    
    // Animation test
    float m_ColorPhase = 0.0f;
};

} // namespace FNF
