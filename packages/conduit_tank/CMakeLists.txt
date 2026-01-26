cmake_minimum_required(VERSION 3.16)
project(conduit_tank LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find conduit_core
find_package(conduit_core REQUIRED)

# MCAP dependency (header-only library)
include(FetchContent)
FetchContent_Declare(
    mcap
    GIT_REPOSITORY https://github.com/foxglove/mcap.git
    GIT_TAG releases/cpp/v1.4.0
)
FetchContent_MakeAvailable(mcap)
set(MCAP_INCLUDE_DIR ${mcap_SOURCE_DIR}/cpp/mcap/include)

# Compression libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(LZ4 REQUIRED liblz4)
pkg_check_modules(ZSTD REQUIRED libzstd)

# Library
add_library(conduit_tank
    src/tank.cpp
)

target_include_directories(conduit_tank
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${MCAP_INCLUDE_DIR}
        ${LZ4_INCLUDE_DIRS}
        ${ZSTD_INCLUDE_DIRS}
)

target_link_libraries(conduit_tank
    PUBLIC
        conduit_core::conduit_core
    PRIVATE
        ${LZ4_LIBRARIES}
        ${ZSTD_LIBRARIES}
)

# Install
install(TARGETS conduit_tank
    EXPORT conduit_tankTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/conduit_tank
    DESTINATION include
)

install(EXPORT conduit_tankTargets
    FILE conduit_tankTargets.cmake
    NAMESPACE conduit_tank::
    DESTINATION lib/cmake/conduit_tank
)

# Config file
include(CMakePackageConfigHelpers)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/conduit_tankConfig.cmake
"include(CMakeFindDependencyMacro)
find_dependency(conduit_core)
include(\${CMAKE_CURRENT_LIST_DIR}/conduit_tankTargets.cmake)
")

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/conduit_tankConfig.cmake
    DESTINATION lib/cmake/conduit_tank
)

# Tests
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    enable_testing()
    find_package(GTest REQUIRED)

    add_executable(tank_test tests/tank_test.cpp)
    target_link_libraries(tank_test conduit_tank GTest::gtest_main)
    add_test(NAME tank_test COMMAND tank_test)
endif()
