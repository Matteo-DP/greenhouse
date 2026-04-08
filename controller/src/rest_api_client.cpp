#include "controller/rest_api_client.hpp"

#include <curl/curl.h>

#include <chrono>
#include <iomanip>
#include <optional>
#include <sstream>

namespace greenhouse {
namespace {

std::string jsonEscape(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (char c : value) {
        switch (c) {
        case '\\':
            escaped += "\\\\";
            break;
        case '"':
            escaped += "\\\"";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped.push_back(c);
            break;
        }
    }
    return escaped;
}

std::string toIso8601Utc(const std::chrono::system_clock::time_point timestamp) {
    const auto t = std::chrono::system_clock::to_time_t(timestamp);
    std::tm tm{};
#ifdef _WIN32
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif

    std::ostringstream out;
    out << std::put_time(&tm, "%FT%TZ");
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

std::optional<std::string> extractIdFromJson(const std::string& json) {
    // Minimal parser for: { ..., "id":"<uuid>", ... }
    const std::string marker = "\"id\"";
    const std::size_t idKey = json.find(marker);
    if (idKey == std::string::npos) {
        return std::nullopt;
    }

    const std::size_t colon = json.find(':', idKey + marker.size());
    if (colon == std::string::npos) {
        return std::nullopt;
    }

    const std::size_t firstQuote = json.find('"', colon + 1);
    if (firstQuote == std::string::npos) {
        return std::nullopt;
    }

    const std::size_t secondQuote = json.find('"', firstQuote + 1);
    if (secondQuote == std::string::npos || secondQuote <= firstQuote + 1) {
        return std::nullopt;
    }

    return json.substr(firstQuote + 1, secondQuote - firstQuote - 1);
}

} // namespace

RestApiClient::RestApiClient(std::string baseUrl, std::shared_ptr<Logger> logger)
    : baseUrl_(std::move(baseUrl)), logger_(std::move(logger)) {
    if (!logger_) {
        logger_ = std::make_shared<Logger>();
    }

    const auto initCode = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (initCode != CURLE_OK) {
        logger_->critical("Failed to initialize libcurl global state.");
    } else {
        logger_->info("REST API client initialized for " + baseUrl_);
    }
}

RestApiClient::~RestApiClient() {
    curl_global_cleanup();
}

std::optional<std::string> RestApiClient::createDevice(const CreateDeviceRequest& request) {
    logger_->debug("Creating device in backend: " + request.name + " [" + deviceTypeToString(request.deviceType) + "]");

    std::ostringstream body;
    body << "{";
    body << "\"name\":\"" << jsonEscape(request.name) << "\",";
    body << "\"description\":\"" << jsonEscape(request.description) << "\",";
    body << "\"device_type\":\"" << deviceTypeToString(request.deviceType) << "\",";
    body << "\"location\":\"" << jsonEscape(request.location) << "\"";
    body << "}";

    std::string response;
    const auto url = baseUrl_ + "/devices/";
    const auto status = postJson(url, body.str(), response, *logger_);
    if (!status.has_value() || *status < 200 || *status >= 300) {
        if (!status.has_value()) {
            logger_->error("POST /devices/ failed for " + request.name + ": libcurl error.");
        } else {
            logger_->warning("POST /devices/ returned HTTP " + std::to_string(*status) + " for " + request.name);
        }
        return std::nullopt;
    }

    const auto remoteId = extractIdFromJson(response);
    if (!remoteId.has_value()) {
        logger_->warning("POST /devices/ succeeded but response did not contain an id for " + request.name);
        return std::nullopt;
    }

    logger_->info("Created remote device " + request.name + " with id " + *remoteId);
    return remoteId;
}

bool RestApiClient::postSensorReading(const std::string& remoteSensorId, const SensorReading& reading) {
    logger_->debug("Posting sensor reading for " + remoteSensorId);

    std::ostringstream valueStr;
    valueStr << std::fixed << std::setprecision(6) << reading.value;

    std::ostringstream body;
    body << "{";
    body << "\"sensor_id\":\"" << jsonEscape(remoteSensorId) << "\",";
    body << "\"time\":\"" << toIso8601Utc(reading.timestamp) << "\",";
    body << "\"value\":" << valueStr.str();
    body << "}";

    std::string response;
    const auto status = postJson(baseUrl_ + "/sensor-readings/", body.str(), response, *logger_);
    if (!status.has_value()) {
        logger_->error("POST /sensor-readings failed for sensor " + remoteSensorId + ": libcurl error.");
        return false;
    }

    if (*status < 200 || *status >= 300) {
        logger_->warning("POST /sensor-readings/ returned HTTP " + std::to_string(*status) + " for sensor " + remoteSensorId);
        return false;
    }

    logger_->info("Posted sensor reading for sensor " + remoteSensorId);
    return true;
}

} // namespace greenhouse
