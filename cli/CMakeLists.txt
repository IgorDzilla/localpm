cmake_minimum_required(VERSION 3.20)

include(FetchContent)
FetchContent_Declare(
        CLI11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG        v2.4.2
)
FetchContent_MakeAvailable(CLI11)
add_library(cli INTERFACE)

target_sources(cli
        INTERFACE
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