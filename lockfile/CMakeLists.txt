cmake_minimum_required(VERSION 3.20)

include(FetchContent)
FetchContent_Declare(
  tomlplusplus
  GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
  GIT_TAG v3.4.0)
FetchContent_MakeAvailable(tomlplusplus)

add_library(lockfile STATIC src/lockfile.cpp)

target_include_directories(lockfile PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                                           ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(lockfile PUBLIC project_logging
                                      tomlplusplus::tomlplusplus)
