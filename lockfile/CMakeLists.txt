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
    lockfile.cpp
    lockfile.hpp
    lockfile_structure.hpp
)

# Public headers are in the current directory
target_include_directories(lockfile PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(lockfile PUBLIC
    tomlplusplus::tomlplusplus
)

target_compile_features(lockfile PUBLIC cxx_std_17)

target_compile_options(lockfile PRIVATE -Wall -Wextra -Wpedantic)

