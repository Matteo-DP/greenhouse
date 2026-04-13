#include "controller/rest_api_client.hpp"
#include "controller/runtime.hpp"

int main() {
    using namespace greenhouse;

	auto logger = std::make_shared<Logger>(LogLevel::Debug);
	auto apiClient = std::make_shared<RestApiClient>("http://localhost:8000", logger);
    SensorRuntime runtime(apiClient, logger);
	
	if (!runtime.getAndBindNewRemoteSensors()) {
		logger->error("Failed to fetch and bind remote sensors");
		return 1;
	}

	const std::size_t pushedCount = runtime.pollAllOnce();
	logger->info("Captured readings for " + std::to_string(pushedCount) + " sensor(s).");	
	
	const std::size_t flushedCount = runtime.flushReadingsForAllDevices();
	logger->info("Flushed " + std::to_string(flushedCount) + " readings to API.");

	return 0;
}
