cmake_minimum_required(VERSION 3.16)
project(localpm LANGUAGES CXX)

# можно 17, но я поставлю 20 — вдруг позже пригодится
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMNADS ON)

include(FetchContent)

FetchContent_Declare(
  CLI11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
  GIT_TAG        v2.4.2
) 
# Вариант B (без git): раскомментируй эти 2 строки и закомментируй блок выше
# FetchContent_Declare(CLI11
#   URL https://github.com/CLIUtils/CLI11/archive/refs/tags/v2.4.2.tar.gz)
FetchContent_MakeAvailable(CLI11)

add_executable(localpm
  src/main.cpp
)

# наши заголовки (registry.hpp, commands/*.hpp, commands_all.hpp)
target_include_directories(localpm PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# CLI11 даёт интерфейсную цель, линковать к ней достаточно
target_link_libraries(localpm PRIVATE CLI11::CLI11)

if (MSVC)
  target_compile_options(localpm PRIVATE /W4 /permissive-)
else()
  target_compile_options(localpm PRIVATE -Wall -Wextra -Wpedantic)
endif()

