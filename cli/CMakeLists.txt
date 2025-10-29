cmake_minimum_required(VERSION 3.16)
project(cli11_hello LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  CLI11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
  GIT_TAG v2.4.2   # при желании обновите версию
)
FetchContent_MakeAvailable(CLI11)

add_executable(cli11-hello src/main.cpp)
target_link_libraries(cli11-hello PRIVATE CLI11::CLI11)

