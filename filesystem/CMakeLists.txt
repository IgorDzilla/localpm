cmake_minimum_required(VERSION 3.20)
project(test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Нужна системная sqlite3 (dev-пакет)
find_package(SQLite3 REQUIRED)

include(FetchContent)
FetchContent_Declare(sqlite_orm
  GIT_REPOSITORY https://github.com/fnc12/sqlite_orm.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(sqlite_orm)

add_executable(test
  src/main.cpp
)

# Подключаем заголовки sqlite_orm (header-only) и линкуем sqlite3
target_include_directories(test PRIVATE
  ${sqlite_orm_SOURCE_DIR}/include
)
target_link_libraries(test PRIVATE SQLite::SQLite3)

# Варнинги
target_compile_options(test PRIVATE -Wall -Wextra -Wpedantic)

