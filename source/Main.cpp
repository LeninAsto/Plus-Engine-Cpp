/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Main Entry Point
 * 
 * This is the main entry point of the game.
 * Initializes the Application and runs the game loop.
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#define SDL_MAIN_HANDLED

#include "core/Logger.h"
#include "core/Application.h"

int main(int argc, char* argv[]) {
    // Initialize logger
    FNF::Logger::Init(true, "fnf_cpp.log");
    FNF::Logger::SetMinLevel(FNF::LogLevel::DEBUG);
    
    // Create application configuration
    FNF::ApplicationConfig config;
    config.title = "Friday Night Funkin' Plus Engine - C++ Edition";
    config.windowWidth = 1280;
    config.windowHeight = 720;
    config.targetFPS = 60;
    config.vsync = true;
    config.fullscreen = false;
    config.resizable = false;
    
    // Get application instance
    auto& app = FNF::Application::Get();
    
    // Initialize
    if (!app.Init(config)) {
        FNF::Logger::Fatal("Failed to initialize application");
        FNF::Logger::Shutdown();
        return -1;
    }
    
    // Run game loop
    app.Run();
    
    // Cleanup
    app.Shutdown();
    FNF::Logger::Shutdown();
    
    return 0;
}
