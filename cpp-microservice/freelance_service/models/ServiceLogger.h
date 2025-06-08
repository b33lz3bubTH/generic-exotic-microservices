#pragma once
#include <string>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>

enum class LogLevel { INFO, WARN, ERROR };

class ServiceLogger {
public:
    static void log(LogLevel level, const std::string& msg) {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::string levelStr;
        switch (level) {
            case LogLevel::INFO: levelStr = "INFO"; break;
            case LogLevel::WARN: levelStr = "WARN"; break;
            case LogLevel::ERROR: levelStr = "ERROR"; break;
        }
        std::cout << std::put_time(std::localtime(&now_c), "%FT%T%z")
                  << " [" << levelStr << "] " << msg << std::endl;
    }
};
