cmake_minimum_required(VERSION 3.16)
project(conduit_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
find_package(conduit_core REQUIRED)

# Publisher
add_executable(demo-publisher src/publisher.cpp)
target_link_libraries(demo-publisher conduit_core::conduit_core)

# Subscriber
add_executable(demo-subscriber src/subscriber.cpp)
target_link_libraries(demo-subscriber conduit_core::conduit_core)

# Install
install(TARGETS
    demo-publisher
    demo-subscriber
    RUNTIME DESTINATION bin
)
