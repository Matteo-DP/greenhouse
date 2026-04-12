#include "controller/generic_sensor.hpp"
#include "controller/light_sensor.hpp"
#include "controller/moisture_sensor.hpp"
#include "controller/runtime.hpp"
#include "utils.cpp"

#include <nlohmann/json.hpp>

#include <memory>
#include <optional>
#include <string>

namespace {

std::optional<greenhouse::DeviceType> parseRemoteDeviceType(const std::string& deviceType) {
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


std::unique_ptr<greenhouse::Device> makeLocalDeviceFromRemote(const nlohmann::json& remoteDevice) {
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
    if (firmware.empty()) {
        return nullptr;
    }
    if (!deviceType.has_value()) {
        return nullptr;
    }

    using greenhouse::DeviceType;
    switch (*deviceType) {
    case DeviceType::SENSOR:
        return std::make_unique<greenhouse::GenericSensor>(
            deviceId, name, location, unit.empty() ? "unknown" : unit, firmware);
    case DeviceType::MOISTURE:
        return std::make_unique<greenhouse::MoistureSensor>(deviceId, name, location, firmware);
    case DeviceType::LIGHT:
        return std::make_unique<greenhouse::LightSensor>(deviceId, name, location, firmware);
    }

    return nullptr;
}

bool devicesMatchRemote(const greenhouse::Device& device, const nlohmann::json& remoteDevice) {
    using namespace Utils;

    const auto remoteType = parseRemoteDeviceType(jsonStringOrEmpty(remoteDevice, "device_type"));
    if (!remoteType.has_value()) {
        return false;
    }

    if (device.type() != *remoteType) {
        return false;
    }

    return device.name() == jsonStringOrEmpty(remoteDevice, "name")
        && device.location() == jsonStringOrEmpty(remoteDevice, "location");
}

} // namespace

namespace greenhouse {

bool SensorRuntime::bindSensor(
    std::string localDeviceId,
    std::string remoteSensorId,
    std::unique_ptr<HardwareSensor> hardware) {
    if (localDeviceId.empty() || remoteSensorId.empty() || !hardware) {
        return false;
    }

    const Device* device = controller_.findDevice(localDeviceId);
    if (!device) {
        return false;
    }

    auto* sensor = dynamic_cast<const SensorDevice*>(device);
    if (!sensor) {
        return false;
    }

    bindings_[std::move(localDeviceId)] = SensorBinding{
        .remoteSensorId = std::move(remoteSensorId),
        .hardware = std::move(hardware),
    };

    return true;
}

bool SensorRuntime::getAndBindNewRemoteSensors() {
    using namespace Utils;

    const auto devicesJson = apiClient_.getDevices();
    if (!devicesJson.has_value()) {
        return false;
    }

    const auto parsed = nlohmann::json::parse(*devicesJson, nullptr, false);
    if (parsed.is_discarded() || !parsed.is_array()) {
        return false;
    }

    bool changed = false;

    for (const auto& remoteDevice : parsed) {
        if (!remoteDevice.is_object()) {
            continue;
        }

        const auto remoteId = jsonStringOrEmpty(remoteDevice, "id");
        if (remoteId.empty()) {
            continue;
        }

        const Device* matchingDevice = nullptr;
        for (const auto* localDevice : controller_.listDevices()) {
            if (devicesMatchRemote(*localDevice, remoteDevice)) {
                matchingDevice = localDevice;
                break;
            }
        }

        if (matchingDevice) {
            auto bindingIt = bindings_.find(matchingDevice->id());
            if (bindingIt != bindings_.end() && bindingIt->second.remoteSensorId != remoteId) {
                bindingIt->second.remoteSensorId = remoteId;
                changed = true;
            }
            continue;
        }

        if (controller_.findDevice(remoteId)) {
            continue;
        }

        auto localDevice = makeLocalDeviceFromRemote(remoteDevice);
        if (!localDevice) {
            continue;
        }

        if (controller_.registerDevice(std::move(localDevice))) {
            changed = true;
        }
    }

    return changed;
}

bool SensorRuntime::pollOnce(const std::string& localDeviceId) {
    const auto bindingIt = bindings_.find(localDeviceId);
    if (bindingIt == bindings_.end()) {
        return false;
    }

    double value = 0.0;
    if (!bindingIt->second.hardware->read(value)) {
        return false;
    }

    if (!controller_.recordSensorReading(localDeviceId, value)) {
        return false;
    }

    const auto reading = controller_.latestReading(localDeviceId);
    if (!reading.has_value()) {
        return false;
    }

    return apiClient_.postSensorReading(bindingIt->second.remoteSensorId, *reading);
}

std::size_t SensorRuntime::pollAllOnce() {
    std::size_t successCount = 0;
    for (const auto& [localDeviceId, _] : bindings_) {
        if (pollOnce(localDeviceId)) {
            ++successCount;
        }
    }
    return successCount;
}

} // namespace greenhouse
