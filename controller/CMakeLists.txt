cmake_minimum_required(VERSION 3.16)
project(greenhouse_controller LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)

add_library(greenhouse_controller
    src/controller.cpp
    src/logger.cpp
    src/runtime.cpp
    src/rest_api_client.cpp
)

target_include_directories(greenhouse_controller PUBLIC include)
target_link_libraries(greenhouse_controller PUBLIC CURL::libcurl)

add_executable(greenhouse_controller_demo src/main.cpp)
target_link_libraries(greenhouse_controller_demo PRIVATE greenhouse_controller)
