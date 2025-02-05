cmake_minimum_required(VERSION 3.25.1)
project(resonate)

set(CMAKE_C_STANDARD 11)

set(SOURCES
    src/main.c
    src/animation.c
    src/hue_dtls_client.c
    src/hue_rest_client.c
    src/hue_stream_message.c
)

add_executable(resonate)

target_sources(resonate PRIVATE ${SOURCES})

target_include_directories(resonate PRIVATE include)

target_compile_options(resonate PRIVATE
    -Wall
    -Wextra
    -Werror
)

target_include_directories(resonate PRIVATE "/usr/include/mbedtls")
target_link_libraries(resonate PRIVATE
    "/usr/lib/x86_64-linux-gnu/libmbedtls.so"
    "/usr/lib/x86_64-linux-gnu/libmbedcrypto.so"
)

find_package(CURL REQUIRED)
target_link_libraries(resonate PRIVATE CURL::libcurl)
