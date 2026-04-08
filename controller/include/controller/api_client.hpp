#pragma once

#include "controller/device_type.hpp"
#include "controller/sensor_reading.hpp"

#include <optional>
#include <string>

namespace greenhouse {

struct CreateDeviceRequest {
    std::string name;
    std::string description;
    DeviceType deviceType;
    std::string location;
};

class ApiClient {
public:
    virtual ~ApiClient() = default;

    // POST /devices
    // Returns new remote UUID if successful.
    virtual std::optional<std::string> createDevice(const CreateDeviceRequest& request) = 0;

    // POST /sensor-readings with body: {sensor_id, time, value}
    virtual bool postSensorReading(const std::string& remoteSensorId, const SensorReading& reading) = 0;
};

} // namespace greenhouse
