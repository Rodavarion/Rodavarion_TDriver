#pragma once

#include <mutex>
#include <string_view>

namespace rodavarion::core {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class Logger final {
public:
    static Logger& instance();

    void setMinimumLevel(LogLevel level) noexcept;
    void log(LogLevel level, std::string_view message);

private:
    Logger() = default;

    static const char* levelName(LogLevel level) noexcept;

    std::mutex mutex_;
    LogLevel minimumLevel_{LogLevel::Info};
};

} // namespace rodavarion::core
