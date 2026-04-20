#pragma once

#include "controller/hardware_sensor.hpp"
#include "controller/logger.hpp"

#include <memory>
#include <unordered_map>
#include <functional>
#include <stdexcept>

namespace greenhouse {

class FirmwareFactory {
public:
    static FirmwareFactory& getInstance() {
        static FirmwareFactory instance;
        return instance;
    }

    void registerClass(const std::string& name, std::function<std::unique_ptr<HardwareSensor>()> creator) {
        registry[name] = creator;
    }

    std::unique_ptr<HardwareSensor> create(const std::string& name) {
        if (name.empty()) {
            throw std::runtime_error("Firmware name cannot be empty");
        }

        auto it = registry.find(name);
        if (it != registry.end()) {
            Logger::getInstance().debug("Creating hardware sensor for firmware: " + name);
            return it->second();
        }
        throw std::runtime_error("Firmware not found: " + name);
    }

private:    
    FirmwareFactory();

    std::unordered_map<std::string, std::function<std::unique_ptr<HardwareSensor>()>> registry;
};

} // namespace greenhouse