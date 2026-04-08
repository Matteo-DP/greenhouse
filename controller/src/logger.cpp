#include "controller/logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace greenhouse {
namespace {

std::string levelToString(const LogLevel level) {
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

Logger::Logger(LogLevel minimumLevel) : minimumLevel_(minimumLevel) {}

void Logger::setMinimumLevel(const LogLevel minimumLevel) {
    std::lock_guard lock(mutex_);
    minimumLevel_ = minimumLevel;
}

void Logger::debug(const std::string& message) {
    log(LogLevel::Debug, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::Info, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::Warning, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::Error, message);
}

void Logger::critical(const std::string& message) {
    log(LogLevel::Critical, message);
}

void Logger::log(const LogLevel level, const std::string& message) {
    std::lock_guard lock(mutex_);
    if (!shouldLog(level, minimumLevel_)) {
        return;
    }

    auto& stream = (level == LogLevel::Debug || level == LogLevel::Info) ? std::clog : std::cerr;
    stream << '[' << currentTimestamp() << "] [" << levelToString(level) << "] " << message << '\n';
}

} // namespace greenhouse
