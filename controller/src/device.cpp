#include "controller/device.hpp"
#include "controller/generic_sensor.hpp"
#include "controller/light_sensor.hpp"
#include "controller/moisture_sensor.hpp"

namespace greenhouse {

std::unique_ptr<Device> Device::fromJson(const nlohmann::json& remoteDevice) {
    if (!remoteDevice.contains("id") || !remoteDevice["id"].is_string()) {
        return nullptr;
    }

    using namespace Utils;
    const auto deviceId = remoteDevice["id"].get<std::string>();
    const auto name = jsonStringOrEmpty(remoteDevice, "name");
    const auto location = jsonStringOrEmpty(remoteDevice, "location");
    const auto typeValue = jsonStringOrEmpty(remoteDevice, "device_type");
    const auto unit = jsonStringOrEmpty(remoteDevice, "unit");
    const auto deviceType = parseRemoteDeviceType(typeValue);
    const std::string firmware = jsonStringOrEmpty(remoteDevice, "firmware");

    if (firmware.empty() || !deviceType.has_value()) {
        return nullptr;
    }

    switch (*deviceType) {
    case DeviceType::SENSOR:
        return std::make_unique<GenericSensor>(
            deviceId, name, location, unit.empty() ? "unknown" : unit, firmware);
    case DeviceType::MOISTURE:
        return std::make_unique<MoistureSensor>(deviceId, name, location, firmware);
    case DeviceType::LIGHT:
        return std::make_unique<LightSensor>(deviceId, name, location, firmware);
    }

    return nullptr;
}

} // namespace greenhouse