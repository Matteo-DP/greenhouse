#pragma once

#include "controller/device.hpp"
#include "controller/sensor_reading.hpp"

#include <optional>
#include <vector>

namespace greenhouse {

class SensorDevice : public Device {
public:
    SensorDevice(std::string id, std::string name, std::string location, std::string firmware)
        : Device(std::move(id), std::move(name), std::move(location), std::move(firmware)) {}

    ~SensorDevice() override = default;

    virtual bool validateValue(double value) const noexcept = 0;
    [[nodiscard]] virtual std::string unit() const = 0;

    bool recordReading(double value, std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now()) {
        if (!validateValue(value) || !enabled()) {
            return false;
        }

        SensorReading reading;
        reading.deviceId = id(); // TODO: is this local id?
        // store readings based on database Id 
        // reading.deviceId = remoteDeviceId;
        reading.timestamp = timestamp;
        reading.value = value;
        reading.unit = unit();
        readings_.push_back(std::move(reading));
        return true;
    }

    [[nodiscard]] std::optional<SensorReading> latestReading() const {
        if (readings_.empty()) {
            return std::nullopt;
        }
        return readings_.back();
    }

    [[nodiscard]] const std::vector<SensorReading>& readings() const noexcept { return readings_; }

private:
    std::vector<SensorReading> readings_;
};

} // namespace greenhouse
