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
    // using namespace greenhouse;

    // GreenhouseController controller;
    // RestApiClient apiClient("http://localhost:8000");
    // SensorRuntime runtime(controller, apiClient);

    // controller.registerDevice(std::make_unique<GenericSensor>(
    //     "sensor-temp-1",
    //     "Ambient Temperature",
    //     "Greenhouse A",
    //     "C",
    //     -20.0,
    //     80.0));

    // controller.registerDevice(std::make_unique<MoistureSensor>(
    //     "sensor-moisture-1",
    //     "Soil Moisture",
    //     "Bed 2"));

    // controller.registerDevice(std::make_unique<LightSensor>(
    //     "sensor-light-1",
    //     "Sunlight Sensor",
    //     "North Wall"));

    // const std::size_t pushedCount = runtime.pollAllOnce();
    // std::cout << "Pushed readings to API for " << pushedCount << " sensor(s).\n";

    // for (const auto* device : controller.listDevices()) {
    //     const auto reading = controller.latestReading(device->id());
    //     if (!reading) {
    //         continue;
    //     }

    //     std::cout
    //         << device->id() << " | "
    //         << device->name() << " | "
    //         << reading->value << " " << reading->unit << " | "
    //         << formatTimestamp(reading->timestamp)
    //         << '\n';
    // }

    return 0;
}
