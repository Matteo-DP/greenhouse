#include "controller/rest_api_client.hpp"
#include "controller/runtime.hpp"

int main() {
    using namespace greenhouse;

	auto& logger = Logger::getInstance();
	logger.setMinimumLevel(LogLevel::Debug);
	auto apiClient = std::make_shared<RestApiClient>("http://localhost:8000");
    SensorRuntime runtime(apiClient);
	
	if (!runtime.getAndBindNewRemoteSensors()) {
		logger.error("Failed to fetch and bind remote sensors");
		return 1;
	}
	
	logger.debug(runtime.toString());

	const std::size_t pushedCount = runtime.pollAllOnce();
	logger.info("Captured readings for " + std::to_string(pushedCount) + " sensor(s).");	
	
	const std::size_t flushedCount = runtime.flushReadingsForAllDevices();
	logger.info("Flushed " + std::to_string(flushedCount) + " readings to API.");

	return 0;
}
