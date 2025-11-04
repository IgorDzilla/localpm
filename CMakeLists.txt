cmake_minimum_required(VERSION 3.20)

project(localpm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

option(LOCALPM_BUILD_TESTS "Build tests" ON)
option(LOCALPM_BUILD_APPS "Build apps (binaries in apps/)" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Core/headers (if any)
if (EXISTS "${CMAKE_SOURCE_DIR}/src/CMakeLists.txt")
    add_subdirectory(src)
endif()

# Optional modules from other branch
# IMPORTANT: lockfile must be built before cli since cli depends on it
if (EXISTS "${CMAKE_SOURCE_DIR}/lockfile/CMakeLists.txt")
    add_subdirectory(lockfile)
endif()
if (EXISTS "${CMAKE_SOURCE_DIR}/cli/CMakeLists.txt")
    add_subdirectory(cli)
endif()

# Applications
if (LOCALPM_BUILD_APPS AND EXISTS "${CMAKE_SOURCE_DIR}/apps/CMakeLists.txt")
    add_subdirectory(apps)
endif()

# Tests
if (LOCALPM_BUILD_TESTS AND EXISTS "${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(tests)
endif()
