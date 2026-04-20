#include "controller/logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace greenhouse {
namespace {


std::string currentTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    std::ostringstream out;
    out << std::put_time(&tm, "%FT%T");
    return out.str();
}

bool shouldLog(const LogLevel messageLevel, const LogLevel minimumLevel) {
    return static_cast<int>(messageLevel) >= static_cast<int>(minimumLevel);
}

} // namespace

std::shared_ptr<ApiClient> Logger::apiClient_ = nullptr;

Logger::Logger() : minimumLevel_(LogLevel::Debug) {}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

bool Logger::flushAndPushLogs(std::shared_ptr<ApiClient> apiClient) {
    if (apiClient->postLogs(logs)) {
        logs.clear();
        return true;
    }
    return false;
}

std::string Logger::levelToString(const LogLevel level) {
    switch (level) {
    case LogLevel::Debug:
        return "DEBUG";
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warning:
        return "WARNING";
    case LogLevel::Error:
        return "ERROR";
    case LogLevel::Critical:
        return "CRITICAL";
    }
    return "INFO";
}

void Logger::setMinimumLevel(const LogLevel minimumLevel) {
    std::lock_guard lock(mutex_);
    minimumLevel_ = minimumLevel;
}

void Logger::debug(const std::string& message, bool noSaveLog) {
    log(LogLevel::Debug, message, noSaveLog);
}

void Logger::info(const std::string& message, bool noSaveLog) {
    log(LogLevel::Info, message, noSaveLog);
}

void Logger::warning(const std::string& message, bool noSaveLog) {
    log(LogLevel::Warning, message, noSaveLog);
}

void Logger::error(const std::string& message, bool noSaveLog) {
    log(LogLevel::Error, message, noSaveLog);
}

void Logger::critical(const std::string& message, bool noSaveLog) {
    log(LogLevel::Critical, message, noSaveLog);
}

void Logger::log(const LogLevel level, const std::string& message, bool noSaveLog) {
    std::lock_guard lock(mutex_);
    if (!shouldLog(level, minimumLevel_)) {
        return;
    }

    auto& stream = (level == LogLevel::Debug || level == LogLevel::Info) ? std::clog : std::cerr;
    stream << '[' << currentTimestamp() << "] [" << levelToString(level) << "] " << message << '\n';
    logs.push_back({level, message, std::chrono::system_clock::now()});

    if (apiClient_ && !noSaveLog && (
        logs.size() >= 10 || 
        level == LogLevel::Critical || 
        (logs.size() > 2 && std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - logs.front().timestamp).count() > 30)
    )) { 
        lock.~lock_guard(); // unlock before API call to avoid deadlocks if API client calls back into logger (lmao)
        // this causes an infinite feedback loop, not good lmao
        // just noSaveLog = true whenever logging postLog errors
        flushAndPushLogs(apiClient_);
    }
}

} // namespace greenhouse
