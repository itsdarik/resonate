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

target_include_directories(resonate PRIVATE "/opt/homebrew/Cellar/mbedtls/3.6.2/include")
target_link_libraries(resonate PRIVATE
    "/opt/homebrew/Cellar/mbedtls/3.6.2/lib/libmbedtls.dylib"
    "/opt/homebrew/Cellar/mbedtls/3.6.2/lib/libmbedcrypto.dylib"
)

find_package(CURL REQUIRED)
target_link_libraries(resonate PRIVATE CURL::libcurl)
