
add_library(menu STATIC
    boundfunction.c
    menu.c
    menubutton.c
    signal.c
)
target_compile_features(menu PRIVATE c_std_99)
set_target_properties(menu PROPERTIES C_EXTENSIONS OFF)
target_include_directories(menu PRIVATE "${PROJECT_BINARY_DIR}/include")
target_link_libraries(menu
    PUBLIC
        SDL2::SDL2
        linkedlist
    PRIVATE
        util
        SDL2_ttf::SDL2_ttf
)
