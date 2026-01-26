cmake_minimum_required(VERSION 3.16)
project(conduit_flow LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(conduit_core REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)

add_library(conduit_flow
    src/parser.cpp
    src/executor.cpp
)

target_include_directories(conduit_flow
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${YAMLCPP_INCLUDE_DIRS}
)

target_link_libraries(conduit_flow
    PUBLIC
        conduit_core::conduit_core
    PRIVATE
        ${YAMLCPP_LIBRARIES}
)

# Install
install(TARGETS conduit_flow
    EXPORT conduit_flowTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/conduit_flow
    DESTINATION include
)

install(EXPORT conduit_flowTargets
    FILE conduit_flowTargets.cmake
    NAMESPACE conduit_flow::
    DESTINATION lib/cmake/conduit_flow
)

include(CMakePackageConfigHelpers)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/conduit_flowConfig.cmake
"include(CMakeFindDependencyMacro)
find_dependency(conduit_core)
include(\${CMAKE_CURRENT_LIST_DIR}/conduit_flowTargets.cmake)
")

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/conduit_flowConfig.cmake
    DESTINATION lib/cmake/conduit_flow
)

# Tests
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    enable_testing()
    find_package(GTest REQUIRED)

    add_executable(flow_test tests/flow_test.cpp)
    target_link_libraries(flow_test conduit_flow GTest::gtest_main ${YAMLCPP_LIBRARIES})
    target_include_directories(flow_test PRIVATE ${YAMLCPP_INCLUDE_DIRS})
    add_test(NAME flow_test COMMAND flow_test)
endif()
