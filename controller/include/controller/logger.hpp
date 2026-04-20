#pragma once

#include "controller/log_entry.hpp"
#include "controller/api_client.hpp"

#include <chrono>
#include <vector>
#include <mutex>
#include <string>
#include <memory>

namespace greenhouse {

class Logger {
public:
    static Logger& getInstance();

    bool flushAndPushLogs(std::shared_ptr<ApiClient> apiClient);

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    void setMinimumLevel(LogLevel minimumLevel);
    static std::string levelToString(const LogLevel level);

    void debug(const std::string& message, bool noSaveLog = false);
    void info(const std::string& message, bool noSaveLog = false);
    void warning(const std::string& message, bool noSaveLog = false);
    void error(const std::string& message, bool noSaveLog = false);
    void critical(const std::string& message, bool noSaveLog = false);

    static void setApiClient(std::shared_ptr<ApiClient> apiClient) {
        apiClient_ = std::move(apiClient);
    }

private:
    Logger();

    static std::shared_ptr<ApiClient> apiClient_;

    void log(LogLevel level, const std::string& message, bool noSaveLog = false);

    LogLevel minimumLevel_;
    std::mutex mutex_;

    std::vector<LogEntry> logs;
};

} // namespace greenhouse
