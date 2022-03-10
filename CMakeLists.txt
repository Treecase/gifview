cmake_minimum_required(VERSION 3.10)

project(gifview VERSION 0.1.0)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# extra warnings
add_compile_options(-Wall -Wextra)

configure_file("src/config.h.in" "config.h")

set(srcs
    "src/main.c"
    "src/gif.c"
    "src/util.c"
    "src/lzw.c"
    "src/args.c"
    "src/keybinds.c"
)
add_executable(${PROJECT_NAME} ${srcs})

target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")

# link SDL2
target_link_libraries(${PROJECT_NAME} SDL2)
