#pragma once

#include <chrono>
#include <string>

namespace greenhouse {

struct SensorReading {
    std::string deviceId;
    std::chrono::system_clock::time_point timestamp;
    double value{0.0};
};

} // namespace greenhouse
