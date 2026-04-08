#include "controller/controller.hpp"

namespace greenhouse {

bool GreenhouseController::registerDevice(std::unique_ptr<Device> device) {
    if (!device) {
        return false;
    }

    std::lock_guard lock(mutex_);
    const auto& id = device->id();
    if (devices_.find(id) != devices_.end()) {
        return false;
    }

    devices_.emplace(id, std::move(device));
    return true;
}

bool GreenhouseController::removeDevice(const std::string& deviceId) {
    std::lock_guard lock(mutex_);
    return devices_.erase(deviceId) > 0;
}

std::vector<const Device*> GreenhouseController::listDevices() const {
    std::lock_guard lock(mutex_);

    std::vector<const Device*> result;
    result.reserve(devices_.size());

    for (const auto& [_, device] : devices_) {
        result.push_back(device.get());
    }

    return result;
}

std::vector<const Device*> GreenhouseController::listDevicesByType(DeviceType type) const {
    std::lock_guard lock(mutex_);

    std::vector<const Device*> result;
    for (const auto& [_, device] : devices_) {
        if (device->type() == type) {
            result.push_back(device.get());
        }
    }

    return result;
}

const Device* GreenhouseController::findDevice(const std::string& deviceId) const {
    std::lock_guard lock(mutex_);
    const auto it = devices_.find(deviceId);
    if (it == devices_.end()) {
        return nullptr;
    }
    return it->second.get();
}

bool GreenhouseController::recordSensorReading(
    const std::string& deviceId,
    double value,
    std::chrono::system_clock::time_point timestamp) {
    std::lock_guard lock(mutex_);
    const auto it = devices_.find(deviceId);
    if (it == devices_.end()) {
        return false;
    }

    auto* sensor = dynamic_cast<SensorDevice*>(it->second.get());
    if (!sensor) {
        return false;
    }

    return sensor->recordReading(value, timestamp);
}

std::optional<SensorReading> GreenhouseController::latestReading(const std::string& deviceId) const {
    std::lock_guard lock(mutex_);
    const auto it = devices_.find(deviceId);
    if (it == devices_.end()) {
        return std::nullopt;
    }

    const auto* sensor = dynamic_cast<const SensorDevice*>(it->second.get());
    if (!sensor) {
        return std::nullopt;
    }

    return sensor->latestReading();
}

} // namespace greenhouse
