#pragma once

#include "controller/api_client.hpp"
#include "controller/logger.hpp"

#include <string>

namespace greenhouse {

class RestApiClient final : public ApiClient {
public:
    explicit RestApiClient(std::string baseUrl, Logger& logger = Logger::getInstance());
    ~RestApiClient() override;

    std::optional<std::string> getDevices() override;
    // std::optional<std::string> createDevice(const CreateDeviceRequest& request) override;
    bool postSensorReading(const std::string& remoteSensorId, const SensorReading& reading) override;

    // GET /devices to fetch all devices, then for any new sensors, bind them to local hardware sensors if possible. This allows dynamic addition of devices from the backend without needing to restart the controller.
private:
    std::string baseUrl_;
    Logger& logger_;
};

} // namespace greenhouse
