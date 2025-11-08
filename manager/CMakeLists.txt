cmake_minimum_required(VERSION 3.20)

include(FetchContent)
include(FetchContent)
FetchContent_Declare(
  SQLiteCpp
  GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
  GIT_TAG 3.3.1)

FetchContent_MakeAvailable(SQLiteCpp)

add_library(manager STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/database.cpp)
target_include_directories(manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                                          ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(manager PUBLIC SQLiteCpp)
