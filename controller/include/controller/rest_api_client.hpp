#pragma once

#include "controller/api_client.hpp"
#include "controller/logger.hpp"

#include <memory>
#include <string>

namespace greenhouse {

class RestApiClient final : public ApiClient {
public:
    explicit RestApiClient(std::string baseUrl, std::shared_ptr<Logger> logger = std::make_shared<Logger>());
    ~RestApiClient() override;

    std::optional<std::string> createDevice(const CreateDeviceRequest& request) override;
    bool postSensorReading(const std::string& remoteSensorId, const SensorReading& reading) override;

private:
    std::string baseUrl_;
    std::shared_ptr<Logger> logger_;
};

} // namespace greenhouse
