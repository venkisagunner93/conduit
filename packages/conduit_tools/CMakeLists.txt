cmake_minimum_required(VERSION 3.16)
project(conduit_tools LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
find_package(conduit_core REQUIRED)
find_package(conduit_tank REQUIRED)
find_package(conduit_flow REQUIRED)

# Single conduit binary
add_executable(conduit
    src/main.cpp
    src/cmd_topics.cpp
    src/cmd_info.cpp
    src/cmd_echo.cpp
    src/cmd_hz.cpp
    src/cmd_record.cpp
    src/cmd_flow.cpp
)

target_include_directories(conduit PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(conduit
    conduit_core::conduit_core
    conduit_tank::conduit_tank
    conduit_flow::conduit_flow
)

install(TARGETS conduit RUNTIME DESTINATION bin)
install(FILES completion/conduit.bash DESTINATION share/conduit/completion)
