#include "controller/runtime.hpp"

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
