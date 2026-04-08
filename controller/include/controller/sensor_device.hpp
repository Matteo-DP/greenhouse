#pragma once

#include "controller/device.hpp"
#include "controller/sensor_reading.hpp"

#include <optional>
#include <vector>

namespace greenhouse {

class SensorDevice : public Device {
public:
    SensorDevice(std::string id, std::string name, std::string location)
        : Device(std::move(id), std::move(name), std::move(location)) {}

    ~SensorDevice() override = default;

    virtual bool validateValue(double value) const noexcept = 0;
    [[nodiscard]] virtual std::string unit() const = 0;

    bool recordReading(double value, std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now()) {
        if (!validateValue(value) || !enabled()) {
            return false;
        }

        readings_.push_back(SensorReading{.deviceId = id(), .timestamp = timestamp, .value = value, .unit = unit()});
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
