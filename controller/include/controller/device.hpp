#pragma once

#include "controller/device_type.hpp"

#include <nlohmann/json.hpp>
#include <string>
#include "controller/utils.hpp"

namespace greenhouse {

class Device {
public:
    Device(std::string id, std::string name, std::string location, std::string firmware)
        : id_(std::move(id)), name_(std::move(name)), location_(std::move(location)), firmware_(std::move(firmware)) {}

    virtual ~Device() = default;

    [[nodiscard]] const std::string& id() const noexcept { return id_; }
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] const std::string& location() const noexcept { return location_; }
    [[nodiscard]] const std::string& firmware() const noexcept { return firmware_; }

    [[nodiscard]] bool enabled() const noexcept { return enabled_; }
    void setEnabled(bool enabled) noexcept { enabled_ = enabled; }

    [[nodiscard]] virtual DeviceType type() const noexcept = 0;
    
    [[nodiscard]] virtual std::string toString() const {
        std::string str;
        str += "Device ID: " + id() + "\n";
        str += "Name: " + name() + "\n";
        str += "Location: " + location() + "\n";
        str += "Firmware: " + firmware() + "\n";
        str += "Type: " + deviceTypeToString(type()) + "\n";
        return str;
    };
    
    // Device factory
    // [[nodiscard]] static std::unique_ptr<Device> fromJson(const nlohmann::json& remoteDevice);

    // compare with remote Device
    bool operator==(const nlohmann::json& remoteDevice) const {
        using namespace Utils;

        const auto remoteType = greenhouse::parseRemoteDeviceType(jsonStringOrEmpty(remoteDevice, "device_type"));
        if (!remoteType.has_value()) {
            return false;
        }

        if (this->type() != *remoteType) {
            return false;
        }

        return this->name() == jsonStringOrEmpty(remoteDevice, "name");
    }
    

private:
    std::string id_;
    std::string name_;
    std::string location_;
    std::string firmware_;
    bool enabled_{true};
};

} // namespace greenhouse
