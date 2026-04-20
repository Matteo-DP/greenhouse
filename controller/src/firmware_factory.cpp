#include "controller/firmware_factory.hpp"
#include "firmware/mock.hpp"

greenhouse::FirmwareFactory::FirmwareFactory() {
    this->registerClass(static_cast<std::string>(firmware::Mock::name), []() {
        return std::make_unique<firmware::Mock>();
    });
}