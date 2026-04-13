cmake_minimum_required(VERSION 3.16)
project(greenhouse_controller LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

find_package(CURL REQUIRED)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(nlohmann_json)

add_library(greenhouse_controller_lib
    src/controller.cpp
    src/device.cpp
    src/logger.cpp
    src/runtime.cpp
    src/rest_api_client.cpp
)

target_include_directories(greenhouse_controller_lib PUBLIC include)
target_link_libraries(greenhouse_controller_lib PUBLIC CURL::libcurl nlohmann_json::nlohmann_json)

add_executable(greenhouse_controller_demo src/main-demo.cpp)
target_link_libraries(greenhouse_controller_demo PRIVATE greenhouse_controller_lib)
add_executable(greenhouse_controller src/main.cpp)
target_link_libraries(greenhouse_controller PRIVATE greenhouse_controller_lib)
