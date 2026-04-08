#pragma once

#include "controller/device_type.hpp"

#include <string>

namespace greenhouse {

class Device {
public:
    Device(std::string id, std::string name, std::string location)
        : id_(std::move(id)), name_(std::move(name)), location_(std::move(location)) {}

    virtual ~Device() = default;

    [[nodiscard]] const std::string& id() const noexcept { return id_; }
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] const std::string& location() const noexcept { return location_; }

    [[nodiscard]] bool enabled() const noexcept { return enabled_; }
    void setEnabled(bool enabled) noexcept { enabled_ = enabled; }

    [[nodiscard]] virtual DeviceType type() const noexcept = 0;

private:
    std::string id_;
    std::string name_;
    std::string location_;
    bool enabled_{true};
};

} // namespace greenhouse
