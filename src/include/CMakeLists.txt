
add_subdirectory(gif)
add_subdirectory(linkedlist)

add_library(util STATIC util.c)
target_include_directories(util PUBLIC .)
target_link_libraries(util PUBLIC SDL2::SDL2)

target_include_directories(gif PUBLIC .)
target_include_directories(linkedlist PUBLIC .)
