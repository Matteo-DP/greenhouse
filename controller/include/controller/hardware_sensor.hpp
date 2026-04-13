#pragma once

namespace greenhouse {

class HardwareSensor {
public:
    virtual ~HardwareSensor() = default;

    // Returns true when a value was successfully read from hardware.
    virtual bool read(double& outValue) = 0;
    
    virtual bool init() = 0;
};

} // namespace greenhouse
