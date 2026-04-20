#pragma once

#include "controller/device_type.hpp"
#include "controller/log_entry.hpp"
#include "controller/sensor_reading.hpp"

#include <optional>
#include <vector>
#include <string>

namespace greenhouse {

class ApiClient {
public:
    virtual ~ApiClient() = default;

    // GET /devices to fetch all devices, then for any new sensors, bind them to local hardware sensors if possible. This allows dynamic addition of devices from the backend without needing to restart the controller.
    virtual std::optional<std::string> getDevices() = 0;

    // // POST /devices
    // // Returns new remote UUID if successful.
    // virtual std::optional<std::string> createDevice(const CreateDeviceRequest& request) = 0;

    // POST /sensor-readings with body: {sensor_id, time, value}
    virtual bool postSensorReading(const std::string& remoteSensorId, const SensorReading& reading) = 0;
    virtual bool postSensorReadings(const std::vector<SensorReading>& readings) = 0;

    virtual bool postLogs(const std::vector<LogEntry>& logs) = 0;
};

} // namespace greenhouse
