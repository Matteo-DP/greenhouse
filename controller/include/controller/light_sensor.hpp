#pragma once

#include "controller/sensor_device.hpp"

namespace greenhouse {

class LightSensor final : public SensorDevice {
public:
    LightSensor(std::string id, std::string name, std::string location)
        : SensorDevice(std::move(id), std::move(name), std::move(location)) {}

    [[nodiscard]] DeviceType type() const noexcept override { return DeviceType::LIGHT; }

    [[nodiscard]] bool validateValue(double value) const noexcept override {
        return value >= 0.0;
    }

    [[nodiscard]] std::string unit() const override { return "lux"; }
};

} // namespace greenhouse
