cmake_minimum_required(VERSION 3.20)

project(localpm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- compile commands ---
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CC_DIRS
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/cli"
  "${CMAKE_SOURCE_DIR}/cli/include"
  "${CMAKE_SOURCE_DIR}/lockfile"
  "${CMAKE_SOURCE_DIR}/lockfile/include"
)

add_custom_target(cc-symlinks ALL
  COMMENT "Create symlinks to build/compile_commands.json"
)

foreach(dir IN LISTS CC_DIRS)
  add_custom_command(TARGET cc-symlinks POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${dir}"
    COMMAND ${CMAKE_COMMAND} -E rm -f "${dir}/compile_commands.json"
    COMMAND ${CMAKE_COMMAND} -E create_symlink
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${dir}/compile_commands.json"
    )
endforeach()
# --- compile commands ---

# --- Logger --

include(FetchContent)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.14.1 # или актуальную
)
FetchContent_MakeAvailable(spdlog) # даст таргет spdlog::spdlog

add_library(project_logging INTERFACE)
target_link_libraries(project_logging INTERFACE spdlog::spdlog)
target_include_directories(project_logging INTERFACE
  ${CMAKE_SOURCE_DIR}/src/include
)

target_compile_definitions(project_logging INTERFACE SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO)
# --- Logger ---


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
