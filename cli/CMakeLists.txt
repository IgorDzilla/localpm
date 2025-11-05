cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Fetch CLI11 library
include(FetchContent)
FetchContent_Declare(
    CLI11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG        v2.4.2
)
FetchContent_MakeAvailable(CLI11)

# Create cli library (header-only/interface library)
# This is an INTERFACE library because all commands are header-only
# and use inline registration pattern
add_library(cli INTERFACE)

target_sources(cli
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}     
)

target_include_directories(cli
    INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src/include
        ${CMAKE_SOURCE_DIR}/lockfile
)

target_link_libraries(cli INTERFACE
    CLI11::CLI11
    project_logging
    lockfile
)

target_compile_features(cli INTERFACE cxx_std_20)
