#pragma once

#include "controller/api_client.hpp"
#include "controller/logger.hpp"

#include <string>

namespace greenhouse {

class RestApiClient final : public ApiClient {
public:
    static RestApiClient& getInstance() {
        if (baseUrl_.empty()) {
            throw std::runtime_error("Base URL not set for RestApiClient");
        }
        static RestApiClient instance;
        return instance;
    } 

    static void setBaseUrl(const std::string& url) {
        RestApiClient::baseUrl_ = url;
    }

    ~RestApiClient() override;

    // GET /devices to fetch all devices, then for any new sensors, bind them to local hardware sensors if possible. This allows dynamic addition of devices from the backend without needing to restart the controller.
    std::optional<std::string> getDevices() override;
    // std::optional<std::string> createDevice(const CreateDeviceRequest& request) override;
    bool postSensorReading(const std::string& remoteSensorId, const SensorReading& reading) override;

private:
    static std::string baseUrl_;

    explicit RestApiClient();
};

} // namespace greenhouse
