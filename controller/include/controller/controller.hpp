#pragma once

#include "controller/device.hpp"
#include "controller/sensor_device.hpp"
#include "controller/sensor_reading.hpp"

#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace greenhouse {

class GreenhouseController {
public:
    bool registerDevice(std::unique_ptr<Device> device);
    bool removeDevice(const std::string& deviceId);

    [[nodiscard]] std::vector<const Device*> listDevices() const;
    [[nodiscard]] std::vector<const Device*> listDevicesByType(DeviceType type) const;

    [[nodiscard]] const Device* findDevice(const std::string& deviceId) const;

    bool recordSensorReading(
        const std::string& deviceId,
        double value,
        std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now());

    [[nodiscard]] std::optional<SensorReading> latestReading(const std::string& deviceId) const;

private:
    std::unordered_map<std::string, std::unique_ptr<Device>> devices_;
    mutable std::mutex mutex_;
};

} // namespace greenhouse
