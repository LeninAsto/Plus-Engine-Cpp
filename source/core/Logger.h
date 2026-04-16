/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Logger System
 * 
 * Provides console and file logging with different severity levels.
 * Thread-safe logging for multithreaded environments.
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <sstream>
#include <ctime>

namespace FNF {

enum class LogLevel {
    TRACE,    // Detailed debug info
    DEBUG,    // Debug information
    INFO,     // General information
    WARN,     // Warnings
    ERROR,    // Errors
    FATAL     // Fatal errors (program will likely crash)
};

class Logger {
public:
    /**
     * Initialize the logger
     * @param enableFileLogging Enable writing to log file
     * @param logFilePath Path to log file (default: "fnf_cpp.log")
     */
    static void Init(bool enableFileLogging = true, const std::string& logFilePath = "fnf_cpp.log");
    
    /**
     * Shutdown the logger (close file handles)
     */
    static void Shutdown();
    
    /**
     * Set minimum log level (messages below this won't be logged)
     */
    static void SetMinLevel(LogLevel level);
    
    /**
     * Log a message
     */
    static void Log(LogLevel level, const std::string& message);
    
    /**
     * Convenience methods
     */
    static void Trace(const std::string& message) { Log(LogLevel::TRACE, message); }
    static void Debug(const std::string& message) { Log(LogLevel::DEBUG, message); }
    static void Info(const std::string& message)  { Log(LogLevel::INFO, message); }
    static void Warn(const std::string& message)  { Log(LogLevel::WARN, message); }
    static void Error(const std::string& message) { Log(LogLevel::ERROR, message); }
    static void Fatal(const std::string& message) { Log(LogLevel::FATAL, message); }
    
private:
    static std::ofstream s_LogFile;
    static std::mutex s_LogMutex;
    static LogLevel s_MinLevel;
    static bool s_FileLoggingEnabled;
    
    /**
     * Get current timestamp as string
     */
    static std::string GetTimestamp();
    
    /**
     * Get log level as colored string
     */
    static std::string GetLevelString(LogLevel level, bool colored = true);
    
    /**
     * Get ANSI color code for log level
     */
    static const char* GetColorCode(LogLevel level);
};

} // namespace FNF
