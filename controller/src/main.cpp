#include <thread>
#include <chrono>

#include "controller/rest_api_client.hpp"
#include "controller/runtime.hpp"

int main() {
    using namespace greenhouse;

	auto& logger = Logger::getInstance();
	logger.setMinimumLevel(LogLevel::Debug);
	auto apiClient = std::make_shared<RestApiClient>("http://localhost:8000");
	Logger::setApiClient(apiClient);
    SensorRuntime runtime(apiClient);

	apiClient->testConnection();
	
	if (!runtime.getAndBindNewRemoteSensors()) {
		logger.error("Failed to fetch and bind remote sensors");
		return 1;
	}
	
	logger.debug(runtime.toString());

	while (true) {
		std::size_t pushedCount = runtime.pollAllOnce();
		std::this_thread::sleep_for(std::chrono::steady_clock::duration(std::chrono::milliseconds(500)));
		pushedCount = runtime.pollAllOnceMaybeFlush();
		
		// const std::size_t flushedCount = runtime.flushReadingsForAllDevices();

		std::this_thread::sleep_for(std::chrono::steady_clock::duration(std::chrono::milliseconds(500)));
	}

	return 0;
}
