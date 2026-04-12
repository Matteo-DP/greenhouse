#pragma once

#include "controller/sensor_device.hpp"

namespace greenhouse {

class MoistureSensor final : public SensorDevice {
public:
    MoistureSensor(std::string id, std::string name, std::string location, std::string firmware)
        : SensorDevice(std::move(id), std::move(name), std::move(location), std::move(firmware)) {}

    [[nodiscard]] DeviceType type() const noexcept override { return DeviceType::MOISTURE; }

    [[nodiscard]] bool validateValue(double value) const noexcept override {
        return value >= 0.0 && value <= 100.0;
    }

    [[nodiscard]] std::string unit() const override { return "%"; }
};

} // namespace greenhouse
