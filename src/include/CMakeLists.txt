
add_library(util OBJECT util.c)

add_subdirectory(gif)
add_subdirectory(linkedlist)

target_include_directories(gif PUBLIC .)
target_include_directories(linkedlist PUBLIC .)
target_include_directories(util PUBLIC .)

target_link_libraries(util PUBLIC SDL2::SDL2)
