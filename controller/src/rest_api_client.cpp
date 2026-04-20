#include "controller/rest_api_client.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <iomanip>
#include <optional>
#include <sstream>

namespace greenhouse {
namespace {

std::string toIso8601Utc(std::chrono::high_resolution_clock::time_point timestamp) {
    using namespace std::chrono;

    // Split into seconds + remainder
    auto seconds = time_point_cast<std::chrono::seconds>(timestamp);
    auto ms = duration_cast<std::chrono::milliseconds>(timestamp - seconds).count();

    // Convert seconds to time_t
    std::time_t t = high_resolution_clock::to_time_t(seconds);

    std::tm tm{};
#ifdef _WIN32
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif

    std::ostringstream out;
    out << std::put_time(&tm, "%FT%T")
        << '.' << std::setw(3) << std::setfill('0') << ms
        << 'Z';

    return out.str();
}

std::string deviceTypeToString(const DeviceType type) {
    switch (type) {
    case DeviceType::SENSOR:
        return "SENSOR";
    case DeviceType::MOISTURE:
        return "MOISTURE";
    case DeviceType::LIGHT:
        return "LIGHT";
    }
    return "SENSOR";
}

size_t writeToString(void* contents, const size_t size, const size_t nmemb, void* userp) {
    const size_t bytes = size * nmemb;
    auto* response = static_cast<std::string*>(userp);
    response->append(static_cast<const char*>(contents), bytes);
    return bytes;
}

std::optional<long> postJson(
    const std::string& url,
    const std::string& jsonBody,
    std::string& responseBody,
    Logger& logger) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        logger.critical("curl_easy_init() failed for " + url);
        return std::nullopt;
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(jsonBody.size()));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);

    const auto performCode = curl_easy_perform(curl);
    long httpStatus = 0;
    if (performCode == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpStatus);
    } else {
        logger.error(std::string("curl_easy_perform() failed for ") + url + ": " + curl_easy_strerror(performCode));
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (performCode != CURLE_OK) {
        return std::nullopt;
    }

    return httpStatus;
}

std::optional<long> getJson(
    const std::string& url,
    std::string& responseBody,
    Logger& logger) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        logger.critical("curl_easy_init() failed for " + url);
        return std::nullopt;
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);

    const auto performCode = curl_easy_perform(curl);
    long httpStatus = 0;
    if (performCode == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpStatus);
    } else {
        logger.error(std::string("curl_easy_perform() failed for ") + url + ": " + curl_easy_strerror(performCode));
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (performCode != CURLE_OK) {
        return std::nullopt;
    }

    return httpStatus;
}

std::optional<std::string> extractIdFromJson(const std::string& jsonText) {
    const auto json = nlohmann::json::parse(jsonText, nullptr, false);
    if (json.is_discarded() || !json.contains("id") || !json["id"].is_string()) {
        return std::nullopt;
    }

    return json["id"].get<std::string>();
}

} // namespace

RestApiClient::RestApiClient(std::string baseUrl, Logger& logger)
    : baseUrl_(std::move(baseUrl)), logger_(logger) {

    const auto initCode = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (initCode != CURLE_OK) {
        logger_.critical("Failed to initialize libcurl global state.");
    } else {
        logger_.info("REST API client initialized for " + baseUrl_);
    }
}

RestApiClient::~RestApiClient() {
    curl_global_cleanup();
}

bool RestApiClient::postLogs(const std::vector<LogEntry>& logs) {
    nlohmann::json body = nlohmann::json::array();
    for (const auto& log : logs) {
        body.push_back({
            {"log_level", Logger::levelToString(log.level)},
            {"message", log.message},
            {"timestamp", toIso8601Utc(log.timestamp)},
        });
    };

    std::string response;
    const auto status = postJson(baseUrl_ + "/logs/", body.dump(), response, logger_);
    if (!status.has_value()) {
        logger_.error("POST /logs/ failed: libcurl error.", true);
        return false;
    }

    if (*status < 200 || *status >= 300) {
        logger_.error("POST /logs/ returned HTTP " + std::to_string(*status), true);
        return false;
    }

    logger_.info("Posted " + std::to_string(logs.size()) + " log(s) to API.", true);
    return true;
}

std::optional<std::string> RestApiClient::getDevices() {
    std::string response;
    const auto status = getJson(baseUrl_ + "/devices/", response, logger_);
    if (!status.has_value()) {
        logger_.error("GET /devices/ failed: libcurl error.");
        return std::nullopt;
    }

    if (*status < 200 || *status >= 300) {
        logger_.warning("GET /devices/ returned HTTP " + std::to_string(*status));
        return std::nullopt;
    }

    return response;
}

// std::optional<std::string> RestApiClient::createDevice(const CreateDeviceRequest& request) {
//     logger_->debug("Creating device in backend: " + request.name + " [" + deviceTypeToString(request.deviceType) + "]");

//     const nlohmann::json body = {
//         {"name", request.name},
//         {"description", request.description},
//         {"device_type", deviceTypeToString(request.deviceType)},
//         {"location", request.location},
//     };

//     std::string response;
//     const auto url = baseUrl_ + "/devices/";
//     const auto status = postJson(url, body.dump(), response, *logger_);
//     if (!status.has_value() || *status < 200 || *status >= 300) {
//         if (!status.has_value()) {
//             logger_->error("POST /devices/ failed for " + request.name + ": libcurl error.");
//         } else {
//             logger_->warning("POST /devices/ returned HTTP " + std::to_string(*status) + " for " + request.name);
//         }
//         return std::nullopt;
//     }

//     const auto remoteId = extractIdFromJson(response);
//     if (!remoteId.has_value()) {
//         logger_->warning("POST /devices/ succeeded but response did not contain an id for " + request.name);
//         return std::nullopt;
//     }

//     logger_->info("Created remote device " + request.name + " with id " + *remoteId);
//     return remoteId;
// }

bool RestApiClient::postSensorReading(const std::string& remoteSensorId, const SensorReading& reading) {
    const nlohmann::json body = {
        {"sensor_id", remoteSensorId},
        {"time", toIso8601Utc(reading.timestamp)},
        {"value", reading.value},
    };

    std::string response;
    const auto status = postJson(baseUrl_ + "/sensor-readings/", body.dump(), response, logger_);
    if (!status.has_value()) {
        logger_.error("POST /sensor-readings failed for sensor " + remoteSensorId + ": libcurl error.");
        return false;
    }

    if (*status < 200 || *status >= 300) {
        logger_.warning("POST /sensor-readings/ returned HTTP " + std::to_string(*status) + " for sensor " + remoteSensorId);
        return false;
    }

    logger_.info("Posted sensor reading for sensor " + remoteSensorId);
    return true;
}

bool RestApiClient::postSensorReadings(const std::vector<SensorReading>& readings) {
    nlohmann::json body = nlohmann::json::array();
    for (const auto& reading : readings) {
        body.push_back({
            {"sensor_id", reading.deviceId},
            {"time", toIso8601Utc(reading.timestamp)},
            {"value", reading.value},
        });
    };

    std::string response;
    const auto status = postJson(baseUrl_ + "/sensor-readings/", body.dump(), response, logger_);
    if (!status.has_value()) {
        logger_.error("POST /sensor-readings/batch/ failed: libcurl error.");
        return false;
    }

    if (*status < 200 || *status >= 300) {
        logger_.warning("POST /sensor-readings/batch/ returned HTTP " + std::to_string(*status));
        return false;
    }

    logger_.info("Posted " + std::to_string(readings.size()) + " sensor reading(s) to API.");
    return true;
}

bool RestApiClient::testConnection() const {
    std::string response;
    const auto status = getJson(baseUrl_ + "/", response, logger_);
    if (!status.has_value()) {
        logger_.error("Connection test failed: libcurl error.");
        return false;
    }

    if (*status < 200 || *status >= 300) {
        logger_.warning("Connection test returned HTTP " + std::to_string(*status));
        return false;
    }

    logger_.info("Connection test succeeded with HTTP " + std::to_string(*status));
    return true;
}

} // namespace greenhouse
