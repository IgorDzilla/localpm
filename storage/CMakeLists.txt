cmake_minimum_required(VERSION 3.20)

add_library(storage STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/storage.cpp)

target_link_libraries(storage PUBLIC semver)
target_include_directories(storage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
