#pragma once

#include "controller/api_client.hpp"
#include "controller/controller.hpp"
#include "controller/hardware_sensor.hpp"
#include "controller/logger.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace greenhouse {

class SensorRuntime {
public:
    SensorRuntime(GreenhouseController& controller, ApiClient& apiClient, std::shared_ptr<Logger> logger)
        : controller_(controller), apiClient_(apiClient), logger_(logger) {}

    bool pollOnce(const std::string& localDeviceId);
    std::size_t pollAllOnce();
    
    // gets from db 
    bool getAndBindNewRemoteSensors();

private:
    struct SensorBinding {
        std::string remoteSensorId;
        std::unique_ptr<SensorDevice> device;
    };

    // localDeviceId: device id in this C++ process
    // remoteSensorId: UUID from backend /devices table
    bool bindSensor(std::string localDeviceId, std::string remoteSensorId, std::unique_ptr<SensorDevice> device);

    GreenhouseController& controller_;
    ApiClient& apiClient_;
    std::unordered_map<std::string, SensorBinding> bindings_; // <localDeviceId, SensorBinding>
    std::shared_ptr<Logger> logger_;
};

} // namespace greenhouse
