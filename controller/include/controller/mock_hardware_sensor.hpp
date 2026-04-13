#pragma once

#include "controller/hardware_sensor.hpp"

namespace greenhouse {

class FixedValueSensor final : public HardwareSensor {
public:
    explicit FixedValueSensor(double value) : value_(value) {}
    
    ~FixedValueSensor() override = default;

    bool read(double& outValue) override {
        outValue = value_;
        return true;
    }
    
    bool init() override {
        return true;
    }

private:
    double value_;
};

} // namespace greenhouse
