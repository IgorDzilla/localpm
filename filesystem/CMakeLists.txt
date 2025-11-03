cmake_minimum_required(VERSION 3.20)
project(test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Нужна системная sqlite3 (dev-пакет)

include(FetchContent)
FetchContent_Declare(
  SQLiteCpp
  GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
  GIT_TAG        3.3.1
)
FetchContent_MakeAvailable(SQLiteCpp)

add_executable(test
  src/main.cpp
)

# Подключаем заголовки sqlite_orm (header-only) и линкуем sqlite3
target_link_libraries(test PRIVATE SQLiteCpp SQLite::SQLite3)

# Варнинги
target_compile_options(test PRIVATE -Wall -Wextra -Wpedantic)

