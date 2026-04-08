#pragma once

#include "controller/hardware_sensor.hpp"

namespace greenhouse {

class FixedValueSensor final : public HardwareSensor {
public:
    explicit FixedValueSensor(double value) : value_(value) {}

    bool read(double& outValue) override {
        outValue = value_;
        return true;
    }

private:
    double value_;
};

} // namespace greenhouse
