#pragma once
#include "controller/hardware_sensor.hpp"

#include <string_view>

namespace firmware {

using namespace greenhouse;

class Mock : public HardwareSensor {
public:
  bool read(double &outValue) override {
    outValue = 69.00;
    return true;
  }

  bool init() override { return true; }

  static constexpr std::string_view name = "mock";
};

} // namespace firmware
