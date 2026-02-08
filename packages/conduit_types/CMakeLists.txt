cmake_minimum_required(VERSION 3.16)
project(conduit_types LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(conduit_types INTERFACE)

target_include_directories(conduit_types
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Install
install(TARGETS conduit_types
    EXPORT conduit_typesTargets
)

install(DIRECTORY include/conduit_types
    DESTINATION include
)

install(EXPORT conduit_typesTargets
    FILE conduit_typesTargets.cmake
    NAMESPACE conduit_types::
    DESTINATION lib/cmake/conduit_types
)

# Config file
include(CMakePackageConfigHelpers)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/conduit_typesConfig.cmake
"include(\${CMAKE_CURRENT_LIST_DIR}/conduit_typesTargets.cmake)
")

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/conduit_typesConfig.cmake
    DESTINATION lib/cmake/conduit_types
)

# Tests
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    enable_testing()
    find_package(GTest REQUIRED)

    add_executable(types_test tests/types_test.cpp)
    target_link_libraries(types_test conduit_types GTest::gtest_main)
    add_test(NAME types_test COMMAND types_test)

    add_executable(primitives_test tests/primitives_test.cpp)
    target_link_libraries(primitives_test conduit_types GTest::gtest_main)
    add_test(NAME primitives_test COMMAND primitives_test)

    add_executable(derived_test tests/geometry_test.cpp)
    target_link_libraries(derived_test conduit_types GTest::gtest_main)
    add_test(NAME derived_test COMMAND derived_test)
endif()
