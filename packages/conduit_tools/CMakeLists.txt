cmake_minimum_required(VERSION 3.16)
project(conduit_tools LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
find_package(conduit_core REQUIRED)
find_package(conduit_tank REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)

# Single conduit binary
add_executable(conduit
    src/main.cpp
    src/cmd_topics.cpp
    src/cmd_info.cpp
    src/cmd_echo.cpp
    src/cmd_hz.cpp
    src/cmd_record.cpp
    src/cmd_launch.cpp
)

target_include_directories(conduit PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${YAMLCPP_INCLUDE_DIRS}
)

target_link_libraries(conduit
    conduit_core::conduit_core
    conduit_tank::conduit_tank
    ${YAMLCPP_LIBRARIES}
)

install(TARGETS conduit RUNTIME DESTINATION bin)
