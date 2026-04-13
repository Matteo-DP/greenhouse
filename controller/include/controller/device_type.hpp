#pragma once

#include <optional>
#include <string>

namespace greenhouse {

enum class DeviceType {
    SENSOR,
    MOISTURE,
    LIGHT,
};

inline std::optional<greenhouse::DeviceType> parseRemoteDeviceType(const std::string& deviceType) {
    using greenhouse::DeviceType;

    if (deviceType == "SENSOR") {
        return DeviceType::SENSOR;
    }
    if (deviceType == "MOISTURE") {
        return DeviceType::MOISTURE;
    }
    if (deviceType == "LIGHT") {
        return DeviceType::LIGHT;
    }
    return std::nullopt;
}

inline std::string deviceTypeToString(DeviceType type) {
    switch (type) {
    case DeviceType::SENSOR:
        return "SENSOR";
    case DeviceType::MOISTURE:
        return "MOISTURE";
    case DeviceType::LIGHT:
        return "LIGHT";
    }
    return "UNKNOWN";
}

} // namespace greenhouse
