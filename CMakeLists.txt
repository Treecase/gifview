cmake_minimum_required(VERSION 3.10)

project(gifview)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# extra warnings
add_compile_options(-Wall -Wextra)

set(srcs
    "src/main.c"
    "src/gif.c"
    "src/util.c"
    "src/lzw.c"
)
add_executable(${PROJECT_NAME} ${srcs})

# link SDL2
target_link_libraries(${PROJECT_NAME} SDL2)
