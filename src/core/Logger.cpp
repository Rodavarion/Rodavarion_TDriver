#include "rodavarion/core/Logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

namespace rodavarion::core {

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::setMinimumLevel(const LogLevel level) noexcept {
    minimumLevel_ = level;
}

void Logger::log(const LogLevel level, const std::string_view message) {
    if (static_cast<int>(level) < static_cast<int>(minimumLevel_)) {
        return;
    }

    const auto now = std::chrono::system_clock::now();
    const std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

    std::lock_guard lock(mutex_);
    std::clog << std::put_time(std::localtime(&nowTime), "%Y-%m-%d %H:%M:%S")
              << " [" << levelName(level) << "] "
              << message << '\n';
}

const char* Logger::levelName(const LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error:   return "ERROR";
    }
    return "UNKNOWN";
}

} // namespace rodavarion::core
