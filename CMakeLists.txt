cmake_minimum_required(VERSION 3.25.1)
project(resonate)

set(CMAKE_C_STANDARD 11)

set(SOURCES
    src/main.c
    src/hue_dtls_client.c
)

add_executable(resonate)

target_sources(resonate PRIVATE ${SOURCES})

target_include_directories(resonate PRIVATE include)

target_compile_options(resonate PRIVATE
    -Wall
    -Wextra
    -Werror
)
