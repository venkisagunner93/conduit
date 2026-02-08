cmake_minimum_required(VERSION 3.16)
project(conduit_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# conduit_types
find_package(conduit_types REQUIRED)

# fmt
include(FetchContent)
set(FMT_INSTALL ON CACHE BOOL "Enable fmt installation")
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 10.2.1
)
FetchContent_MakeAvailable(fmt)

add_library(conduit_core
    src/internal/ring_buffer.cpp
    src/internal/shm_region.cpp
    src/internal/futex.cpp
    src/internal/time.cpp
    src/publisher.cpp
    src/subscriber.cpp
    src/node.cpp
    src/log.cpp
)

target_include_directories(conduit_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(conduit_core
    PUBLIC
        pthread
        fmt::fmt
        conduit_types::conduit_types
    PRIVATE
        rt
)

# Install
install(TARGETS conduit_core
    EXPORT conduit_coreTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/conduit_core
    DESTINATION include
)

install(EXPORT conduit_coreTargets
    FILE conduit_coreTargets.cmake
    NAMESPACE conduit_core::
    DESTINATION lib/cmake/conduit_core
)

# Config file
include(CMakePackageConfigHelpers)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/conduit_coreConfig.cmake
"include(CMakeFindDependencyMacro)
find_dependency(fmt)
find_dependency(conduit_types)
include(\${CMAKE_CURRENT_LIST_DIR}/conduit_coreTargets.cmake)
")

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/conduit_coreConfig.cmake
    DESTINATION lib/cmake/conduit_core
)

# Tests
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    enable_testing()
    find_package(GTest REQUIRED)

    add_executable(ring_buffer_test tests/ring_buffer_test.cpp)
    target_link_libraries(ring_buffer_test conduit_core GTest::gtest_main)
    add_test(NAME ring_buffer_test COMMAND ring_buffer_test)

    add_executable(shm_region_test tests/shm_region_test.cpp)
    target_link_libraries(shm_region_test conduit_core GTest::gtest_main)
    add_test(NAME shm_region_test COMMAND shm_region_test)

    add_executable(futex_test tests/futex_test.cpp)
    target_link_libraries(futex_test conduit_core GTest::gtest_main)
    add_test(NAME futex_test COMMAND futex_test)

    add_executable(pubsub_test tests/pubsub_test.cpp)
    target_link_libraries(pubsub_test conduit_core GTest::gtest_main)
    add_test(NAME pubsub_test COMMAND pubsub_test)

    add_executable(node_test tests/node_test.cpp)
    target_link_libraries(node_test conduit_core GTest::gtest_main)
    add_test(NAME node_test COMMAND node_test)

    add_executable(typed_pubsub_test tests/typed_pubsub_test.cpp)
    target_link_libraries(typed_pubsub_test conduit_core GTest::gtest_main)
    add_test(NAME typed_pubsub_test COMMAND typed_pubsub_test)
endif()
