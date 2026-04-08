#pragma once

#include "controller/api_client.hpp"
#include "controller/controller.hpp"
#include "controller/hardware_sensor.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace greenhouse {

class SensorRuntime {
public:
    SensorRuntime(GreenhouseController& controller, ApiClient& apiClient)
        : controller_(controller), apiClient_(apiClient) {}

    // localDeviceId: device id in this C++ process
    // remoteSensorId: UUID from backend /devices table
    bool bindSensor(std::string localDeviceId, std::string remoteSensorId, std::unique_ptr<HardwareSensor> hardware);

    bool pollOnce(const std::string& localDeviceId);
    std::size_t pollAllOnce();

private:
    struct SensorBinding {
        std::string remoteSensorId;
        std::unique_ptr<HardwareSensor> hardware;
    };

    GreenhouseController& controller_;
    ApiClient& apiClient_;
    std::unordered_map<std::string, SensorBinding> bindings_;
};

} // namespace greenhouse
