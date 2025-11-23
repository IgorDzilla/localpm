cmake_minimum_required(VERSION 3.20)

add_library(database STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/database.cpp)
target_include_directories(database PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                                           ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(database PUBLIC SQLiteCpp semver project_logging)
