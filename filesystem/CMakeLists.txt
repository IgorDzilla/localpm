cmake_minimum_required(VERSION 3.16)
project(test-db LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/SQLiteCpp)

add_executable(main src/main.cpp)
target_link_libraries(main
  SQLiteCpp
  sqlite3
  pthread
  dl
  )


