#pragma once

#include <vector>
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
    static Logger& getInstance();

    bool flushAndPushLogs();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    void setMinimumLevel(LogLevel minimumLevel);

    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);

private:
    Logger();

    void log(LogLevel level, const std::string& message);

    LogLevel minimumLevel_;
    std::mutex mutex_;

    std::vector<std::string> logs;
};

} // namespace greenhouse
