/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Logger System Implementation
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#include "Logger.h"
#include <iomanip>

namespace FNF {

// Static member initialization
std::ofstream Logger::s_LogFile;
std::mutex Logger::s_LogMutex;
LogLevel Logger::s_MinLevel = LogLevel::DEBUG;
bool Logger::s_FileLoggingEnabled = false;

void Logger::Init(bool enableFileLogging, const std::string& logFilePath) {
    s_FileLoggingEnabled = enableFileLogging;
    
    if (s_FileLoggingEnabled) {
        std::lock_guard<std::mutex> lock(s_LogMutex);
        s_LogFile.open(logFilePath, std::ios::out | std::ios::trunc);
        if (s_LogFile.is_open()) {
            s_LogFile << "=========================================" << std::endl;
            s_LogFile << "  FNF Plus Engine - C++ Edition Log" << std::endl;
            s_LogFile << "  Started: " << GetTimestamp() << std::endl;
            s_LogFile << "=========================================" << std::endl;
        } else {
            std::cerr << "[LOGGER] Failed to open log file: " << logFilePath << std::endl;
            s_FileLoggingEnabled = false;
        }
    }
    
    // NOTE: called OUTSIDE the lock to avoid recursive mutex lock
    Info("Logger initialized");
}

void Logger::Shutdown() {
    std::lock_guard<std::mutex> lock(s_LogMutex);
    
    if (s_LogFile.is_open()) {
        s_LogFile << "=========================================" << std::endl;
        s_LogFile << "  Log ended: " << GetTimestamp() << std::endl;
        s_LogFile << "=========================================" << std::endl;
        s_LogFile.close();
    }
}

void Logger::SetMinLevel(LogLevel level) {
    s_MinLevel = level;
}

void Logger::Log(LogLevel level, const std::string& message) {
    // Skip if below minimum level
    if (level < s_MinLevel) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(s_LogMutex);
    
    std::string timestamp = GetTimestamp();
    std::string levelStr = GetLevelString(level, true);
    std::string levelStrPlain = GetLevelString(level, false);
    
    // Console output (with colors)
    std::cout << "[" << timestamp << "] " << levelStr << " " << message << "\033[0m" << std::endl;
    
    // File output (without colors)
    if (s_FileLoggingEnabled && s_LogFile.is_open()) {
        s_LogFile << "[" << timestamp << "] " << levelStrPlain << " " << message << std::endl;
        s_LogFile.flush(); // Ensure it's written immediately
    }
}

std::string Logger::GetTimestamp() {
    auto now = std::time(nullptr);
    
    // Use thread-safe version on Windows
    #ifdef _WIN32
        struct tm timeinfo;
        localtime_s(&timeinfo, &now);
        std::ostringstream oss;
        oss << std::put_time(&timeinfo, "%H:%M:%S");
    #else
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M:%S");
    #endif
    
    return oss.str();
}

std::string Logger::GetLevelString(LogLevel level, bool colored) {
    const char* color = colored ? GetColorCode(level) : "";
    const char* reset = colored ? "\033[0m" : "";
    
    switch (level) {
        case LogLevel::TRACE: return std::string(color) + "[TRACE]" + reset;
        case LogLevel::DEBUG: return std::string(color) + "[DEBUG]" + reset;
        case LogLevel::INFO:  return std::string(color) + "[INFO] " + reset;
        case LogLevel::WARN:  return std::string(color) + "[WARN] " + reset;
        case LogLevel::ERROR: return std::string(color) + "[ERROR]" + reset;
        case LogLevel::FATAL: return std::string(color) + "[FATAL]" + reset;
        default: return "[UNKNOWN]";
    }
}

const char* Logger::GetColorCode(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE: return "\033[37m";      // White
        case LogLevel::DEBUG: return "\033[36m";      // Cyan
        case LogLevel::INFO:  return "\033[32m";      // Green
        case LogLevel::WARN:  return "\033[33m";      // Yellow
        case LogLevel::ERROR: return "\033[31m";      // Red
        case LogLevel::FATAL: return "\033[35;1m";   // Magenta Bold
        default: return "\033[0m";
    }
}

} // namespace FNF
