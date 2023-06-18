
cmake_minimum_required(VERSION 3.13)

file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif()

project(gifview
    VERSION 0.6.0
    DESCRIPTION "GIF viewer in the style of feh."
    LANGUAGES C
)

# Set build type to Release if unset
if(NOT CMAKE_BUILD_TYPE)
    message("Set build type to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()


find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

add_executable(gifview)
target_compile_features(gifview PRIVATE c_std_99)
target_include_directories(gifview PRIVATE "${PROJECT_BINARY_DIR}/include")
target_link_libraries(gifview PRIVATE SDL2::SDL2 SDL2_ttf::SDL2_ttf m)

add_subdirectory(src)


# Install
include(GNUInstallDirs)
install(TARGETS gifview RUNTIME)
install(FILES keys.conf TYPE SYSCONF RENAME gifview/keys.conf)
