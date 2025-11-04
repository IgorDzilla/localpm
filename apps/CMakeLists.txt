cmake_minimum_required(VERSION 3.20)

add_executable(localpm
    main.cpp
)

# Link optional modules if present
if (TARGET cli)
    target_link_libraries(localpm PRIVATE cli)
endif()
if (TARGET lockfile)
    target_link_libraries(localpm PRIVATE lockfile)
endif()

if (MSVC)
    target_compile_options(localpm PRIVATE /W4 /permissive-)
else()
    target_compile_options(localpm PRIVATE -Wall -Wextra -Wpedantic)
endif()
