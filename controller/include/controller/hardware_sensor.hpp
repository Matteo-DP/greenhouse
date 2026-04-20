#pragma once
#include <string>

namespace greenhouse {

class HardwareSensor {
public:
    virtual ~HardwareSensor() = default;

    // Returns true when a value was successfully read from hardware.
    virtual bool read(double& outValue) = 0;
    
    virtual bool init() = 0;

    static constexpr const std::string name = "base";
};

} // namespace greenhouse
