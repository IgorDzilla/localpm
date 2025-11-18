cmake_minimum_required(VERSION 3.20)
project(localpm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(lockfile)
add_subdirectory(cli)
add_subdirectory(database)

include(FetchContent)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.14.1)
FetchContent_MakeAvailable(spdlog) # даст таргет spdlog::spdlog

FetchContent_Declare(
  CLI11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
  GIT_TAG v2.4.2)
FetchContent_MakeAvailable(CLI11)

add_library(project_logging INTERFACE)
target_link_libraries(project_logging INTERFACE spdlog::spdlog)
target_include_directories(project_logging
                           INTERFACE ${CMAKE_SOURCE_DIR}/src/include)

add_executable(localpm src/main.cpp src/include/logger/logger.h)

# Подключаем логгер и include-пути к бинарю
target_link_libraries(localpm PRIVATE project_logging CLI11::CLI11 cli database)

target_include_directories(
  localpm PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/include/
                  ${CMAKE_SOURCE_DIR}/lockfile ${CMAKE_SOURCE_DIR}/cli)

include(CTest)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
