#pragma once

#include "controller/api_client.hpp"
#include "controller/hardware_sensor.hpp"
#include "controller/logger.hpp"
#include "controller/sensor_device.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace greenhouse {

class SensorRuntime {
public:
    SensorRuntime(std::shared_ptr<ApiClient> apiClient, Logger& logger = Logger::getInstance())
        : apiClient_(apiClient), logger_(logger) {}

    [[nodiscard]] std::vector<const SensorDevice*> listDevices() const;
    [[nodiscard]] std::vector<const SensorDevice*> listDevicesByType(DeviceType type) const;
    [[nodiscard]] const SensorDevice* findDevice(const std::string& deviceId) const;

    bool pollOnce(const std::string& localDeviceId);
    std::size_t pollAllOnce();
    std::size_t flushReadingsForDevice(const std::string& localDeviceId);
    std::size_t flushReadingsForAllDevices();
    
    // gets from db 
    bool getAndBindNewRemoteSensors();
    
    [[nodiscard]] const std::string toString() const {
        std::string str;
        str += std::to_string(bindings_.size()) + " bound devices:\n";
        for (const auto& [localId, binding] : bindings_) {
            str += "Local Device ID: " + localId + "\n";
            str += binding.device->toString() + "\n";
        }
        return str;
    }

private:
    struct SensorBinding {
        std::string remoteSensorId;
        std::unique_ptr<SensorDevice> device;
    };

    // localDeviceId: device id in this C++ process
    // remoteSensorId: UUID from backend /devices table
    bool bindSensor(std::string localDeviceId, std::string remoteSensorId, std::unique_ptr<SensorDevice> device);

    // GreenhouseController& controller_;
    std::shared_ptr<ApiClient> apiClient_;
    std::unordered_map<std::string, SensorBinding> bindings_; // <localDeviceId, SensorBinding>
    Logger& logger_;
};

} // namespace greenhouse
