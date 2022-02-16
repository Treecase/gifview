cmake_minimum_required(VERSION 3.10)

project(gifview)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# extra warnings
add_compile_options(-Wall -Wextra)

set(srcs
    "src/main.cpp"
)
add_executable(${PROJECT_NAME} ${srcs})

# link SDL2
target_link_libraries(${PROJECT_NAME} -lSDL2)
