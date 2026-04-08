#include "controller/controller.hpp"
#include "controller/generic_sensor.hpp"
#include "controller/light_sensor.hpp"
#include "controller/mock_hardware_sensor.hpp"
#include "controller/moisture_sensor.hpp"
#include "controller/rest_api_client.hpp"
#include "controller/runtime.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>

namespace {

std::string formatTimestamp(std::chrono::system_clock::time_point tp) {
    const std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    std::ostringstream ss;
    ss << std::put_time(&tm, "%F %T");
    return ss.str();
}

} // namespace

int main() {
    using namespace greenhouse;

    GreenhouseController controller;
    RestApiClient apiClient("http://localhost:8000");
    SensorRuntime runtime(controller, apiClient);

    controller.registerDevice(std::make_unique<GenericSensor>(
        "sensor-temp-1",
        "Ambient Temperature",
        "Greenhouse A",
        "C",
        -20.0,
        80.0));

    controller.registerDevice(std::make_unique<MoistureSensor>(
        "sensor-moisture-1",
        "Soil Moisture",
        "Bed 2"));

    controller.registerDevice(std::make_unique<LightSensor>(
        "sensor-light-1",
        "Sunlight Sensor",
        "North Wall"));

    const auto remoteTempId = apiClient.createDevice(CreateDeviceRequest{
        .name = "Ambient Temperature",
        .description = "Generic sensor",
        .deviceType = DeviceType::SENSOR,
        .location = "Greenhouse A",
    });
    if (remoteTempId == std::nullopt) {
        std::cerr << "Failed to create remote device for temperature sensor.\n";
    }

    const auto remoteMoistureId = apiClient.createDevice(CreateDeviceRequest{
        .name = "Soil Moisture",
        .description = "Moisture probe",
        .deviceType = DeviceType::MOISTURE,
        .location = "Bed 2",
    });
    if (remoteMoistureId == std::nullopt) {
        std::cerr << "Failed to create remote device for moisture sensor.\n";
    }

    const auto remoteLightId = apiClient.createDevice(CreateDeviceRequest{
        .name = "Sunlight Sensor",
        .description = "Light sensor",
        .deviceType = DeviceType::LIGHT,
        .location = "North Wall",
    });
    if (remoteLightId == std::nullopt) {
        std::cerr << "Failed to create remote device for light sensor.\n";
    }

    if (remoteTempId) {
        runtime.bindSensor("sensor-temp-1", *remoteTempId, std::make_unique<FixedValueSensor>(24.8));
    }
    if (remoteMoistureId) {
        runtime.bindSensor("sensor-moisture-1", *remoteMoistureId, std::make_unique<FixedValueSensor>(61.2));
    }
    if (remoteLightId) {
        runtime.bindSensor("sensor-light-1", *remoteLightId, std::make_unique<FixedValueSensor>(18500.0));
    }

    const std::size_t pushedCount = runtime.pollAllOnce();
    std::cout << "Pushed readings to API for " << pushedCount << " sensor(s).\n";

    for (const auto* device : controller.listDevices()) {
        const auto reading = controller.latestReading(device->id());
        if (!reading) {
            continue;
        }

        std::cout
            << device->id() << " | "
            << device->name() << " | "
            << reading->value << " " << reading->unit << " | "
            << formatTimestamp(reading->timestamp)
            << '\n';
    }

    return 0;
}
