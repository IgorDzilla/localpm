cmake_minimum_required(VERSION 3.16)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Fetch tomlplusplus library
include(FetchContent)
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

# Create lockfile library
add_library(lockfile
    src/lockfile.cpp
    include/lockfile.hpp
    lockfile_structure.hpp
)

# Public headers are in the current directory and include/
target_include_directories(lockfile PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(lockfile PUBLIC
    spdlog::spdlog
    tomlplusplus::tomlplusplus
)

target_link_libraries(lockfile PRIVATE project_logging)

target_compile_features(lockfile PUBLIC cxx_std_17)

target_compile_options(lockfile PRIVATE -Wall -Wextra -Wpedantic)
