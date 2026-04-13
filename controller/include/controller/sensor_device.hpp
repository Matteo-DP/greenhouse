#pragma once

#include "controller/device.hpp"
#include "controller/sensor_reading.hpp"
#include "controller/hardware_sensor.hpp"
#include "controller/mock_hardware_sensor.hpp"

#include <optional>
#include <vector>

namespace greenhouse {

class SensorDevice : public Device {
public:
    SensorDevice(std::string id, std::string name, std::string location, std::string firmware)
        : Device(std::move(id), std::move(name), std::move(location), std::move(firmware)) {
            this->hardware_ = SensorDevice::getFirmwareByName(firmware);
        }

    ~SensorDevice() override = default;

    virtual bool validateValue(double value) const noexcept = 0;
    [[nodiscard]] virtual std::string unit() const = 0;
    
    static std::unique_ptr<HardwareSensor> getFirmwareByName(const std::string& firmware) {
        // TODO: actually return different HardwareSensor implementations based on the firmware string. For now, return a dummy one.
        return std::make_unique<FixedValueSensor>();
    }

    bool recordReading() {
        const auto timestamp = std::chrono::system_clock::now();
        std::optional<double> value;
        
        if (this->hardware_ == nullptr) {
            return false;
        }

        if (!this->hardware_->read(*value)) {
            return false;
        }
        
        if (!value.has_value()) {
            return false;
        }

        if (!validateValue(*value) || !enabled()) {
            return false;
        }

        SensorReading reading;
        reading.deviceId = id(); // TODO: is this local id?
        // store readings based on database Id 
        // reading.deviceId = remoteDeviceId;
        reading.timestamp = timestamp;
        reading.value = *value;
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
    [[nodiscard]] const std::unique_ptr<HardwareSensor>& hardware() const noexcept { return hardware_; }

private:
    std::vector<SensorReading> readings_;
    std::unique_ptr<HardwareSensor> hardware_ = nullptr; // points to an implementation of HardwareSensor based on the firmware string
};

} // namespace greenhouse
