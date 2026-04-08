#pragma once

#include "controller/sensor_device.hpp"

#include <optional>

namespace greenhouse {

class GenericSensor final : public SensorDevice {
public:
    GenericSensor(
        std::string id,
        std::string name,
        std::string location,
        std::string readingUnit,
        std::optional<double> minValue = std::nullopt,
        std::optional<double> maxValue = std::nullopt)
        : SensorDevice(std::move(id), std::move(name), std::move(location)),
          readingUnit_(std::move(readingUnit)),
          minValue_(minValue),
          maxValue_(maxValue) {}

    [[nodiscard]] DeviceType type() const noexcept override { return DeviceType::SENSOR; }

    [[nodiscard]] bool validateValue(double value) const noexcept override {
        if (minValue_ && value < *minValue_) {
            return false;
        }
        if (maxValue_ && value > *maxValue_) {
            return false;
        }
        return true;
    }

    [[nodiscard]] std::string unit() const override { return readingUnit_; }

private:
    std::string readingUnit_;
    std::optional<double> minValue_;
    std::optional<double> maxValue_;
};

} // namespace greenhouse
