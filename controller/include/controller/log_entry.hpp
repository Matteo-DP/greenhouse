#pragma once

#include <chrono>
#include <string>

namespace greenhouse {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Critical,
};

struct LogEntry {
    LogLevel level;
    std::string message;
    std::chrono::system_clock::time_point timestamp;
};

} // namespace greenhouse