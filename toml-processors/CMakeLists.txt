cmake_minimum_required(VERSION 3.20)

include(FetchContent)
FetchContent_Declare(
        tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

add_library(toml-processors 
	STATIC
        src/lockfile/lockfile.cpp
)

target_compile_features(toml-processors 
	PUBLIC 
	cxx_std_17
)

target_include_directories(toml-processors
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(toml-processors
        PUBLIC
        project_logging
        tomlplusplus::tomlplusplus
)

#чтобы не править другие CMake
#add_library(lockfile ALIAS toml_processors)


