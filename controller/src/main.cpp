#include "controller/controller.hpp"
#include "controller/rest_api_client.hpp"
#include "controller/runtime.hpp"

int main() {
    using namespace greenhouse;

	auto logger = std::make_shared<Logger>(LogLevel::Debug);
    GreenhouseController controller;
    RestApiClient apiClient("http://localhost:8000", logger);
    SensorRuntime runtime(controller, apiClient, logger);
	
	if (!runtime.getAndBindNewRemoteSensors()) {
		logger->error("Failed to fetch and bind remote sensors");
		return 1;
	}
	
	logger->debug(controller.printDevices());

	const std::size_t pushedCount = runtime.pollAllOnce();
	logger->info("Pushed readings to API for " + std::to_string(pushedCount) + " sensor(s).");	

	return 0;
}
