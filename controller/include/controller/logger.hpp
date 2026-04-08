#pragma once

#include <mutex>
#include <string>

namespace greenhouse {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Critical,
};

class Logger {
public:
    explicit Logger(LogLevel minimumLevel = LogLevel::Debug);

    void setMinimumLevel(LogLevel minimumLevel);

    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);

private:
    void log(LogLevel level, const std::string& message);

    LogLevel minimumLevel_;
    std::mutex mutex_;
};

} // namespace greenhouse
